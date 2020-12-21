// This interface provides a simple library for talking to the supported voice engines.

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sonic.h>
#include "util.h"
#include "speechsw.h"

#define MAX_TEXT_LENGTH (1 << 16)
#define SAMPLE_BUFFER_SIZE 128

struct swEngineSt {
  char *name;
  FILE *fin;
  FILE *fout;
  swCallback callback;
  void *callbackContext;
  sonicStream sonic;
  int16_t *samples;
  uint32_t sampleBufferSize;
  uint32_t sampleRate;
  float speed;
  float pitch;
  int pid;
  bool useSonicPitch;
  bool useSonicSpeed;
  // Volatile because it could be set from a different thread.
  volatile bool cancel;
};

// Write a formatted string to the server.
static void serverPrintf(swEngine engine, const char *format, ...) {
  va_list ap;
  char buf[MAX_TEXT_LENGTH];
  va_start(ap, format);
  vsnprintf(buf, MAX_TEXT_LENGTH - 1, format, ap);
  va_end(ap);
  buf[MAX_TEXT_LENGTH - 1] = '\0';
  swLog("Writing to engine: %s", buf);
  fputs(buf, engine->fin);
  fflush(engine->fin);
}

// Write a string to the server, without adding a newline.
static void serverPuts(swEngine engine, const char *text) {
  swLog("Writing to engine: %s", text);
  fputs(text, engine->fin);
  fflush(engine->fin);
}

// Write "true\n" or "false\n" to the server.
static void writeBool(swEngine engine, bool value) {
  if(value) {
    serverPrintf(engine, "true\n");
  } else {
    serverPrintf(engine, "false\n");

  }
}

// List available engines.
char **swListEngines(const char *enginesDirectory, uint32_t *numEngines) {
  char **engines = swListDirectory(enginesDirectory, numEngines);
  return engines;
}

// Read a line and return true only if the line is "true".
static bool expectTrue(swEngine engine) {
  char *line = swReadLine(engine->fout);
  bool result = !strcmp(line, "true");
  swFree(line);
  return result;
}

// Start the Sonic speed/pitch post-processor.
static void startSonic(swEngine engine) {
  if (engine->sonic == NULL) {
    engine->sonic = sonicCreateStream(engine->sampleRate, 1);
    if (engine->sonic == NULL) {
      fprintf(stderr, "Out of memory");
      exit(1);
    }
  }
}

// Stop the Sonic speed/pitch post-processor.
static void stopSonic(swEngine engine) {
  if (engine->sonic != NULL) {
    sonicDestroyStream(engine->sonic);
    engine->useSonicSpeed = false;
    engine->useSonicPitch = false;
  }
}

// Read a uint32_t from the server.
static uint32_t readUint32(swEngine engine) {
  char *line = swReadLine(engine->fout);
  uint32_t result = atoi(line);
  swFree(line);
  return result;
}

// Create and initialize a new swEngine object, and connect to the speech engine.
swEngine swStart(const char *libDirectory, const char *engineName,
    swCallback callback, void *callbackContext) {
  char *enginesDir =  swSprintf("%s/%s", libDirectory, engineName);
  char *engineExeName = swSprintf("%s/sw_%s", enginesDir, engineName);
  if(!swFileReadable(engineExeName)) {
    fprintf(stderr, "Unable to execute %s\n", engineExeName);
    swFree(enginesDir);
    swFree(engineExeName);
    return NULL;
  }
  swEngine engine = swCalloc(1, sizeof(struct swEngineSt));
  engine->name = swCopyString(engineName);
  engine->callback = callback;
  engine->callbackContext = callbackContext;
  engine->sampleBufferSize = SAMPLE_BUFFER_SIZE;
  engine->samples = swCalloc(engine->sampleBufferSize, sizeof(int16_t));
  engine->pid = swForkWithStdio(engineExeName, &engine->fin, &engine->fout,
    enginesDir, NULL);
  swFree(engineExeName);
  swFree(enginesDir);
  serverPrintf(engine, "get sonicpitch\n");
  engine->useSonicPitch = expectTrue(engine);
  serverPrintf(engine, "get sonicspeed\n");
  engine->useSonicSpeed = expectTrue(engine);
  serverPrintf(engine, "get samplerate\n");
  engine->sampleRate = readUint32(engine);
  if (engine->useSonicSpeed || engine->useSonicPitch) {
    startSonic(engine);
  }
  return engine;
}

// Shut down the speech engine, and free the swEngine object.
void swStop(swEngine engine) {
  serverPrintf(engine, "quit\n");
  fclose(engine->fout);
  fclose(engine->fin);
  swFree(engine->name);
  stopSonic(engine);
  swFree(engine->samples);
  kill(engine->pid, SIGKILL);
  swFree(engine);
}

// Convert a line of hex digits to int16_t.
static uint32_t convertHexToInt16(int16_t *samples, char *line) {
  uint32_t i = 0;
  uint32_t numDigits = 0;
  uint32_t numSamples = 0;
  uint32_t value = 0;
  char c = line[i++];
  while(c != '\0') {
    if(c > ' ') {
      value <<= 4;
      uint32_t digit = 0;
      if(c >= '0' && c <= '9') {
        digit = c - '0';
      } else if(c >= 'A' && c <= 'F') {
        digit = c - 'A' + 10;
      } else if(c >= 'a' && c <= 'f') {
        digit = c - 'a' + 10;
      }
      value += digit;
      numDigits++;
      if(numDigits == 4) {
        samples[numSamples++] = value;
        numDigits = 0;
        value = 0;
      }
    }
    c = line[i++];
  }
  if(numDigits != 0) {
    fprintf(stderr, "Hex digits left over: %u\n", numDigits);
  }
  return numSamples;
}

// Run sonic to adjust speed and/or pitch
static void adjustSamples(swEngine engine, uint32_t *numSamples) {
  sonicStream sonic = engine->sonic;
  sonicWriteShortToStream(sonic, engine->samples, *numSamples);
  *numSamples = sonicSamplesAvailable(sonic);
  if (*numSamples == 0) {
    return;
  }
  if (*numSamples > engine->sampleBufferSize) {
    engine->sampleBufferSize = *numSamples << 1;
    engine->samples = swRealloc(engine->samples, engine->sampleBufferSize, sizeof(int16_t));
  }
  if (sonicReadShortFromStream(sonic, engine->samples, *numSamples) != *numSamples) {
    fprintf(stderr, "Error reading from sonic stream\n");
    exit(1);
  }
}

// Grow the engine's sample buffer to at least bufSize.
static void growSampleBuffer(swEngine engine, uint32_t bufSize) {
  if (engine->sampleBufferSize < bufSize) {
    engine->sampleBufferSize = bufSize << 2;
    engine->samples = swRealloc(engine->samples, engine->sampleBufferSize, sizeof(char));
  }
}

// Read speech data in hexidecimal from the server until "true" is read.
// Caller must free the samples.
static int16_t *readSpeechData(swEngine engine, uint32_t *numSamples) {
  static bool ending = false;  // Used to return flushed samples from Sonic when ending.
  if (ending) {
    ending = false;
    *numSamples = 0;
    return NULL;
  }
  char *line = swReadLine(engine->fout);
  if(!strcmp(line, "true")) {
    swFree(line);
    if (engine->sonic != NULL) {
      // If using Sonic, we're almost, but not quite done.
      sonicFlushStream(engine->sonic);
      *numSamples = sonicSamplesAvailable(engine->sonic);
      if (*numSamples != 0) {
        growSampleBuffer(engine, *numSamples);
        ending = true;
        return engine->samples;
      }
    }
    // We're done.
    *numSamples = 0;
    return NULL;
  }
  uint32_t bufSize  = strlen(line)/4;
  growSampleBuffer(engine, bufSize);
  *numSamples = convertHexToInt16(engine->samples, line);
  swFree(line);
  if (engine->sonic != NULL) {
    adjustSamples(engine, numSamples);
  }
  return engine->samples;
}


// Process speech data from the synth engine untile cancelled or done.
static bool processSpeechData(swEngine engine) {
  uint32_t numSamples;
  int16_t *samples = readSpeechData(engine, &numSamples);
  bool cancelled = false;
  while(samples != NULL && !cancelled) {
    if (numSamples != 0) {
      cancelled = engine->callback(engine, samples, numSamples, engine->cancel,
          engine->callbackContext);
    }
    writeBool(engine, !cancelled);
    samples = readSpeechData(engine, &numSamples);
  }
  // We're done, so signal end of synthesis by sending 0 samples.
  cancelled = engine->callback(engine, samples, 0, engine->cancel, engine->callbackContext);
  return !cancelled;
}

// Synthesize speech samples.  Synthesized samples will be passed to the 
// callback function passed to swStart.  This function blocks until speech
// synthesis is complete.
bool swSpeak(swEngine engine, const char *text, bool isUTF8) {
  // TODO: deal with isUTF8
  // TODO: replace any \n. with \n..
  engine->cancel = false;
  serverPuts(engine, "speak\n");
  serverPuts(engine, text);
  serverPrintf(engine, "\n.\n");
  return processSpeechData(engine);
}

// Synthesize speech samples to speak a single character.  Synthesized samples
// will be passed to the callback function passed to swStart.  This function
// blocks until speech synthesis is complete.
bool swSpeakChar(swEngine engine, const char *utf8Char, size_t bytes) {
  if (utf8Char[bytes] != '\0') {
    swLog("swSpeakChar: No terminating '\0'\n");
    return false;
  }
  bool valid = false;
  uint32_t unicodeChar;
  swFindUTF8LengthAndValidate(utf8Char, bytes, &valid, &unicodeChar);
  if (!valid) {
    swLog("Tried to speak invalid UTF8 char %s\n", utf8Char);
    return false;
  }
  engine->cancel = false;
  serverPrintf(engine, "char %s\n", utf8Char);
  return processSpeechData(engine);
}

// Read a count-prefixed string list from the server.
static char **readStringList(swEngine engine, uint32_t *numStrings) {
  *numStrings = readUint32(engine);
  char **strings = swCalloc(*numStrings, sizeof(char *));
  uint32_t i;
  for(i = 0; i < *numStrings; i++) {
    strings[i] = swReadLine(engine->fout);
  }
  return strings;
}

// Enable/disable using Sonic to set pitch.
void swEnableSonicPitch(swEngine engine, bool enable) {
  if (enable == engine->useSonicPitch) {
    return;
  }
  engine->useSonicPitch = enable;
  if (enable) {
    startSonic(engine);
  } else if (!engine->useSonicSpeed) {
    stopSonic(engine);
  }
  swSetPitch(engine, engine->pitch);
}

// Enable/disable using Sonic to set speed.
void swEnableSonicSpeed(swEngine engine, bool enable) {
  if (enable == engine->useSonicSpeed) {
    return;
  }
  engine->useSonicSpeed = enable;
  if (enable) {
    startSonic(engine);
  } else if (!engine->useSonicPitch) {
    stopSonic(engine);
  }
  swSetSpeed(engine, engine->speed);
}

// Return true of Sonic is currently used to adjust pitch.
bool swSonicUsedForPitch(swEngine engine) {
  return engine->useSonicPitch;
}

// Return true of Sonic is currently used to adjust speed.
bool swUsedForSonicSpeed(swEngine engine) {
  return engine->useSonicSpeed;
}

// Get the sample rate in Hertz.
uint32_t swGetSampleRate(swEngine engine) {
  return engine->sampleRate;
}

// Get a list of supported voices.  The caller can call swFreeStrings
char **swListVoices(swEngine engine, uint32_t *numVoices) {
  serverPrintf(engine, "get voices\n");
  return readStringList(engine, numVoices);
}

// Set the speech speed.  Speed is from -100.0 to 100.0, and 0 is the default.
bool swSetSpeed(swEngine engine, float speed) {
  engine->speed = speed;  // Remember it in case sonic is enabled/disabled.
  if (engine->useSonicSpeed) {
    sonicSetSpeed(engine->sonic, speed);
    return true;
  }
  serverPrintf(engine, "set speed %f\n", speed);
  return expectTrue(engine);
}

// Set the pitch.  0 means default, -100 is min pitch, and 100 is max pitch.
bool swSetPitch(swEngine engine, float pitch) {
  engine->pitch = pitch;  // Remember it in case sonic is enabled/disabled.
  if (engine->useSonicPitch) {
    sonicSetPitch(engine->sonic, pitch);
    return true;
  }
  serverPrintf(engine, "set pitch %f\n", pitch);
  return expectTrue(engine);
}

// Select a voice by it's identifier
bool swSetVoice(swEngine engine, const char *voice) {
  serverPrintf(engine, "set voice %s\n", voice);
  return expectTrue(engine);
}

// Return the engine's native encoding.
swEncoding swGetEncoding(swEngine engine) {
  serverPrintf(engine, "get encoding\n");
  char *line = swReadLine(engine->fout);
  swEncoding encoding = SW_UTF8;
  if(!strcmp(line, "ANSI")) {
    encoding = SW_ANSI;
  }
  swFree(line);
  return encoding;
}

// Interrupt speech while being synthesized.
void swCancel(swEngine engine) {
  engine->cancel = true;
}

// Returns true if swCancel has been called since the last call to swSpeak.
bool swSpeechCanceled(swEngine engine) {
  return engine->cancel;
}

// List available variations on voices.
char **swGetVariants(swEngine engine, uint32_t *numVariants) {
  serverPrintf(engine, "get variants\n");
  return readStringList(engine, numVariants);
}

// Select a voice variant by it's identifier
bool swSetVariant(swEngine engine, const char *variant) {
  serverPrintf(engine, "set variant %s\n", variant);
  return expectTrue(engine);
}

// Set the punctuation level: none, some, most, or all.
bool swSetPunctuation(swEngine engine, swPunctuationLevel level) {
  char *levelName = NULL;
  switch(level) {
  case SW_PUNCT_NONE: levelName = "none"; break;
  case SW_PUNCT_SOME: levelName = "some"; break;
  case SW_PUNCT_MOST: levelName = "most"; break;
  case SW_PUNCT_ALL: levelName = "all"; break;
  default:
    fprintf(stderr, "Unknonwn punctuation level");
    exit(1);
  }
  serverPrintf(engine, "set punctuation %s\n", levelName);
  return expectTrue(engine);
}

// Enable or disable ssml support.
bool swSetSSML(swEngine engine, bool enable) {
  serverPrintf(engine, "set ssml %s\n", enable? "true" : "false");
  return expectTrue(engine);
}

// Return the protocol version, Currently 1 for all engines.
uint32_t swGetVersion(swEngine engine) {
  serverPrintf(engine, "get version\n");
  return readUint32(engine);
}
