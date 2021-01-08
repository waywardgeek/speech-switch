// This interface provides a simple library for talking to the supported voice engines.

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>  // For strcasecmp.
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sonic.h>
#include "util.h"
#include "speechsw.h"

#define MAX_TEXT_LENGTH (1 << 16)
#define SAMPLE_BUFFER_SIZE 128
#define MAX_LANGUAGE_CODE_LEN 4

struct swEngineSt {
  char *name;
  FILE *fin;
  FILE *fout;
  swCallback callback;
  void *callbackContext;
  sonicStream sonic;
  int16_t *samples;
  uint32_t sampleBufferSize;
  char *textBuffer;
  uint32_t textBufferSize;
  uint32_t textBufferPos;
  uint32_t sampleRate;
  swPunctuationLevel punctuationLevel;
  float speed;
  float pitch;
  int pid;
  char languageCode[MAX_LANGUAGE_CODE_LEN];
  bool useSSML;
  bool useSonicPitch;
  bool useSonicSpeed;
  // Volatile because it could be set from a different thread.
  volatile bool cancel;
};

typedef struct {
  uint32_t unicodeChar;
  const char *name;
  swPunctuationLevel punctuationLevel;
} swCharName;

typedef struct {
  const char *languageCode;
  const swCharName *charNames;
  uint32_t numCharNames;
} swPunctuationList;

// These are the substitutions for reading puncutation.  Note that they must be
// sorted, since we do lookup with binary search.
swCharName swEnglishCharNames[] = {
  {'!', "exclamation point", SW_PUNCT_ALL},
  {'"', "double quote", SW_PUNCT_ALL},
  {'#', "number sign", SW_PUNCT_SOME},
  {'$', "dollar sign", SW_PUNCT_SOME},
  {'%', "percent", SW_PUNCT_SOME},
  {'&', "ampersand", SW_PUNCT_SOME},
  {'\'', "single quote", SW_PUNCT_ALL},
  {'(', "open parentheses", SW_PUNCT_MOST},
  {')', "close parentheses", SW_PUNCT_MOST},
  {'*', "star", SW_PUNCT_SOME},
  {'+', "plus", SW_PUNCT_SOME},
  {',', "comma", SW_PUNCT_ALL},
  {'-', "hyphen", SW_PUNCT_MOST},
  {'.', "period", SW_PUNCT_ALL},
  {'/', "slash", SW_PUNCT_SOME},
  {':', "colon", SW_PUNCT_MOST},
  {';', "semicolon", SW_PUNCT_MOST},
  {'<', "less than", SW_PUNCT_SOME},
  {'=', "equals", SW_PUNCT_SOME},
  {'>', "greather than", SW_PUNCT_SOME},
  {'?', "question mark", SW_PUNCT_ALL},
  {'@', "at sign", SW_PUNCT_SOME},
  {'[', "left square bracket", SW_PUNCT_MOST},
  {'\\', "backslash", SW_PUNCT_SOME},
  {']', "right square bracket", SW_PUNCT_MOST},
  {'^', "caret", SW_PUNCT_SOME},
  {'_', "underscore", SW_PUNCT_SOME},
  {'`', "backquote", SW_PUNCT_SOME},
  {'{', "left curly brace", SW_PUNCT_MOST},
  {'|', "vertical bar", SW_PUNCT_SOME},
  {'}', "right curly brace", SW_PUNCT_MOST},
  {'~', "tilde", SW_PUNCT_SOME}
};
swPunctuationList swPunctuation[] = {
  {"en", swEnglishCharNames, sizeof(swEnglishCharNames)/sizeof(swCharName)}
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
  engine->textBufferSize = 42;
  engine->textBuffer = swCalloc(engine->textBufferSize, sizeof(char));
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
  // Default to English.
  strcpy(engine->languageCode, "en");
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
  swFree(engine->textBuffer);
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
    engine->samples = swRealloc(engine->samples, engine->sampleBufferSize, sizeof(int16_t));
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

// Grow the engine's text buffer to at least bufSize.
static void growTextBuffer(swEngine engine, uint32_t bufSize) {
  if (engine->textBufferSize < bufSize) {
    engine->textBufferSize = bufSize << 2;
    engine->textBuffer = swRealloc(engine->textBuffer, engine->textBufferSize, sizeof(char));
  }
}

// Find the current language's punctuation lilst.  Return NULL if not found.
static const swPunctuationList *findCurrentLanguagePunctuationList(swEngine engine) {
  if (engine->languageCode[0] == 0) {
    return NULL;
  }
  for (uint32_t i = 0; i < sizeof(swPunctuation)/sizeof(swPunctuationList); i++) {
    swPunctuationList *punctuationList = swPunctuation + i;
    if (!strcasecmp(punctuationList->languageCode, engine->languageCode)) {
      return punctuationList;
    }
  }
  return NULL;
}

// Find the character in the punctuation list.  Use binary search.  Return NULL
// if not found.
static const swCharName *findCharNameInList(const swPunctuationList *punctuationList,
    uint32_t unicodeChar) {
  uint32_t start = 0;
  uint32_t end = punctuationList->numCharNames;
  do {
    uint32_t middle = (start + end) >> 1;
    const swCharName *charName = punctuationList->charNames + middle;
    if (unicodeChar < charName->unicodeChar) {
      end = middle;
    } else if (unicodeChar > charName->unicodeChar) {
      start = middle + 1;
    } else {
      return charName;
    }
  } while (start < end);
  return NULL;
}

// Add len characters to the text buffer.  Return the new buffer position.
static void addToTextBuffer(swEngine engine, const char *text, uint32_t len) {
  uint32_t pos = engine->textBufferPos;
  if (pos + len + 1 < pos) {
    fprintf(stderr, "Integer overflow in text buffer resize.\n");
    exit (1);
  }
  growTextBuffer(engine, pos + len + 1);
  memcpy(engine->textBuffer + pos, text, len);
  engine->textBuffer[pos + len] = '\0';
  engine->textBufferPos += len;
}

// Process text to speak appropriate punctuation.  Write result to
// engine->textBuffer.
static void processPunctuation(swEngine engine, const char *text) {
  const swPunctuationList *punctuationList = findCurrentLanguagePunctuationList(engine);
  if (punctuationList == NULL) {
    // Copy text verbatum.
    growTextBuffer(engine, strlen(text) + 1);
    strcpy(engine->textBuffer, text);
    return;
  }
  engine->textBufferPos = 0;
  const char *p = text;
  const char *end = text + strlen(text);
  while (p != end) {
    bool valid;
    uint32_t unicodeChar;
    uint8_t len = swFindUTF8LengthAndValidate(p, end - p, &valid, &unicodeChar);
    if (valid) {
      const swCharName *charName = findCharNameInList(punctuationList, unicodeChar);
      if (charName == NULL) {
        // Not punctuation.  Just copy it.
        addToTextBuffer(engine, p, len);
      } else if (charName->punctuationLevel <= engine->punctuationLevel) {
        // Read the namem of the punctuation character.
        addToTextBuffer(engine, " ", 1);
        addToTextBuffer(engine, charName->name, strlen(charName->name));
        addToTextBuffer(engine, " ", 1);
      } else {
        // Replace the punctuation with a space.
        addToTextBuffer(engine, " ", 1);
      }
    }
    p += len;
  }
}

// Synthesize speech samples.  Synthesized samples will be passed to the
// callback function passed to swStart.  This function blocks until speech
// synthesis is complete.
bool swSpeak(swEngine engine, const char *text, bool isUTF8) {
  // TODO: deal with isUTF8
  // TODO: replace any \n. with \n..
  if (engine->useSSML) {
    // Copy text verbatum.
    growTextBuffer(engine, strlen(text) + 1);
    strcpy(engine->textBuffer, text);
  } else {
    // Replace punctuation based on the punctuation level.
    processPunctuation(engine, text);
  }
  engine->cancel = false;
  serverPuts(engine, "speak\n");
  serverPuts(engine, engine->textBuffer);
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

// The local should be appended to the voice name, e.g. "American
// Enblish,en-US".  Set the expected language, which is used to map punctuation
// to words in that language.
static void updateLanguage(swEngine engine, const char *voice) {
  char *p = strrchr(voice, ',');
  if (p == NULL) {
    return;
  }
  p++;
  char *q = strchr(p, '-');
  if (q == NULL) {
    q = p + strlen(p);  // Make q point to end.
  }
  if (q - p > MAX_LANGUAGE_CODE_LEN - 1) {
    return;  // Language code is too long.
  }
  strncpy(engine->languageCode, p, q - p);
  engine->languageCode[q - p] = '\0';
}

// Select a voice by it's identifier
bool swSetVoice(swEngine engine, const char *voice) {
  updateLanguage(engine, voice);
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
  if (level < SW_PUNCT_NONE || level > SW_PUNCT_ALL) {
    return false;
  }
  engine->punctuationLevel = level;
  return true;
}

// Enable or disable ssml support.
bool swSetSSML(swEngine engine, bool enable) {
  engine->useSSML = enable;
  serverPrintf(engine, "set ssml %s\n", enable? "true" : "false");
  return expectTrue(engine);
}

// Return the protocol version, Currently 1 for all engines.
uint32_t swGetVersion(swEngine engine) {
  serverPrintf(engine, "get version\n");
  return readUint32(engine);
}
