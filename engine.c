/* Stdio server for engines.

This file was written by me, Bill Cox, in 2011, and placed into the public domain.
Feel free to use this file in commercial projects or any other use you wish.

To simplify supporting engines in various languages and compile formats, such as
32-bit vs 64-bit, TTS engines talk to speech-switch through stdin and stdout.
There are just a few simple commands.

These should mostly be pretty self-explanitory, with the exception of speak.
The expectation is that the server is single threaded, and so it will read
characters from the socket until a newline is read, and then execute the
command.  The speak command takes text until a line is read with just '.'.  At
that point, it synthesizes the speech, and every so often writes a line of
encoded samples in hex.  After writing each line, the server should check to see
if there is a command waiting, and if so, is it 'cancel'.  If cancelled,
synthesis should stop.  When synthesis ends, a line with "done" should be
printed.

*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>

#include "engine.h"

#define MAX_LINE_LENGTH (1 << 12)
#define MAX_TEXT_LENGTH (1 << 16)

static uint8_t line[MAX_LINE_LENGTH*2];
static uint8_t word[MAX_LINE_LENGTH*2];
static uint8_t *linePos;
static uint8_t *speechBuffer;
static int speechBufferSize;
static uint8_t *textBuffer;
static int textBufferSize;
static bool useANSI = false;

// Switch to ANSI rather than UTF-8.
void swSwitchToANSI(void) {
  useANSI = true;
}

// Convert a factor that changes pitch or speed, to minRange .. maxRange.
int swFactorToRange(float factor, float minFactor, float maxFactor, int minRange,
    int defaultRange, int maxRange) {
  if (factor > maxFactor) {
    factor = maxFactor;
  }
  if (factor < minFactor) {
    factor = minFactor;
  }
  if (factor >= 1.0f) {
    return defaultRange + (int)((maxRange - defaultRange)*
        ((factor - 1.0f)/(maxFactor - 1.0f)) + 0.5f);
  }
  return defaultRange - (int)((defaultRange - minRange)*
      (1.0f/factor - 1.0f)/(1.0f/minFactor - 1.0f) + 0.5f);
}
    
// Make sure that only valid UTF-8 characters are in the line, and that all
// control characters are gone.
static void validateLine(void) {
  uint8_t *p = line;
  uint8_t *q = line;
  int length;
  bool valid;
  uint8_t *lineEnd = line + sizeof(line);

  while(*p != '\0') {
    if(useANSI) {
      length = 1;
      valid = *p >= ' ';
    } else {
      length = swFindUTF8LengthAndValidate((char*)p, lineEnd - p, &valid, NULL);
    }
    if(valid) {
      while(length--) {
        *q++ = *p++;
      }
    } else {
      p += length;
    }
  }
  *q = '\0';
}

// Read a line.  If it's longer than some outragiously long ammount, truncate it. 
static bool readLineRaw(void) {
  int c = getchar();
  int pos = 0;

  if(c == EOF) {
    return false;
  }
  while(c != '\n' && pos < MAX_LINE_LENGTH - 2) {
    line[pos++] = c;
    c = getchar();
    if(c == EOF) {
      return false;
    }
  }
  line[pos] = '\0';
  return true;
}

// Read a line and validate it, removing control characters and invalid UTF-8 characters.
static bool readLine(void) {
  do {
    if(!readLineRaw()) {
      return false;
    }
    validateLine();
  } while(*line == '\0');
  swLog("Read %s\n", line);
  return true;
}

// Write a formatted string to the client.
static void writeClient(char *format, ...) {
  va_list ap;
  char buf[MAX_TEXT_LENGTH];
  va_start(ap, format);
  vsnprintf(buf, MAX_TEXT_LENGTH - 1, format, ap);
  va_end(ap);
  buf[MAX_TEXT_LENGTH - 1] = '\0';
  swLog("Wrote %s\n", buf);
  puts(buf);
  fflush(stdout);
}

// Write a string to the client.
static void putClient(char *string) {
  swLog("Wrote %s\n", string);
  puts(string);
  fflush(stdout);
}

// Execute the getSampleRate command.
static void execGetSampleRate(void) {
  int sampleRate = swGetSampleRate();

  writeClient("%d", sampleRate);
}

// Write "true" or "false" to the client based on the boolean value passed.
static void writeBool(bool value) {
  if(value) {
    putClient("true");
  } else {
    putClient("false");
  }
}

// Just copy a word from the current line position to the word buffer and return
// a pointer to it.  Return NULL if we are at the end.
static char *readWord(void) {
  uint8_t *w = word;
  uint8_t c = *linePos;

  // Skip spaces.
  while(c == ' ') {
    c = *++linePos;
  }
  if(c == '\0') {
    return NULL;
  }
  while(c != '\0' && c != ' ') {
    *w++ = c;
    c = *++linePos;
  }
  *w = '\0';
  return (char *)word;
}

// Execute the getVoices command.
static void execGetVoices(void) {
  uint32_t numVoices, i;
  char **voices = swGetVoices(&numVoices);

  writeClient("%d", numVoices);
  for(i = 0; i < numVoices; i++) {
    writeClient("%s", voices[i]);
  }
  swFreeStringList(voices, numVoices);
}

// Execute the getVariants command.
static void execGetVoiceVariants(void) {
  uint32_t numVariants, i;
  char **variants = swGetVoiceVariants(&numVariants);

  if(variants == NULL) {
    writeClient("0");
    return;
  }
  writeClient("%d", numVariants);
  for(i = 0; i < numVariants; i++) {
    writeClient("%s", variants[i]);
  }
  swFreeStringList(variants, numVariants);
}

// Execute the setVoice command.
static void execSetVoice(void) {
  char *voiceName = (char *)linePos;

  while(*voiceName == ' ') {
    voiceName++;
  }
  writeBool(*voiceName != '\0' && swSetVoice(voiceName));
}

// Execute the setVariant command.
static void execSetVoiceVariant(void) {
  char *variantName = readWord();

  writeBool(variantName != NULL && swSetVoiceVariant(variantName));
}

// Read a floating point value from the line.
static float readFloat(bool *passed) {
  char *floatString = readWord();
  char *end;
  float value;

  *passed = true;
  if(floatString == NULL) {
    *passed = false;
    return 0.0f;
  }
  value = strtod(floatString, &end);
  if(*end != '\0') {
    *passed = false;
    return 0.0f;
  }
  return value;
}

// Read a boolean value, either "true" or "false".
static bool readBool(bool *passed) {
  char *boolString = readWord();

  *passed = true;
  if(boolString == NULL) {
    *passed = false;
    return false;
  }
  if(!strcasecmp(boolString, "true")) {
    return true;
  }
  if(!strcasecmp(boolString, "false")) {
    return false;
  }
  *passed = false;
  return false;
}

// Execute the setPitch command 
static void execSetPitch(void) {
  bool passed;
  float pitch = readFloat(&passed);

  if(!passed) {
    writeBool(false);
    return;
  }
  writeBool(swSetPitch(pitch));
}

// Execute the setSpeed command 
static void execSetSpeed(void) {
  bool passed;
  float speed = readFloat(&passed);

  if(!passed) {
    writeBool(false);
    return;
  }
  writeBool(swSetSpeed(speed));
}

// Execute the setPunctuation command 
static void execSetPunctuation(void) {
  char *levelString = readWord();
  int level = PUNCT_NONE;

  if(levelString == NULL) {
    writeBool(false);
    return;
  }
  if(!strcasecmp(levelString, "none")) {
    level = PUNCT_NONE;
  } else if(!strcasecmp(levelString, "some")) {
    level = PUNCT_SOME;
  } else if(!strcasecmp(levelString, "most")) {
    level = PUNCT_MOST;
  } else if(!strcasecmp(levelString, "all")) {
    level = PUNCT_ALL;
  } else {
    writeBool(false);
    return;
  }
  writeBool(swSetPunctuationLevel(level));
}

// Execute the setSsml command 
static void execSetSsml(void) {
  bool passed;
  bool value = readBool(&passed);

  if(!passed) {
    writeBool(false);
    return;
  }
  writeBool(swSetSSML(value));
}

// Just read one line at a time into the textBuffer until we see a line with "."
// by itself.  If we see a line starting with two dots, remove the first one.
static bool readText(void) {
  int pos = 0;
  int length;
  char *lineBuf;

  while(readLine()) {
    if(!strcmp((char *)line, ".")) {
      textBuffer[pos] = '\0';
      return true;
    }
    if(pos > MAX_TEXT_LENGTH) {
      return false;
    }
    lineBuf = (char *)line;
    if(!strncmp((char *)line, "..", 2)) {
      lineBuf++;
    }
    length = strlen(lineBuf);
    if(textBufferSize < pos + length + 1) {
      textBufferSize = (pos + length) << 1;
      textBuffer = (uint8_t *)swRealloc(textBuffer, textBufferSize, sizeof(uint8_t));
    }
    strcpy((char *)textBuffer + pos, lineBuf);
    pos += length;
  }
  return false;
}

// Execute a speak command.  This will not return until all speech has been synthesized,
// unless processAudio fails to read "true" from the client after sending speech
// samples. */
static bool execSpeak(void) {
  swLog("entering execSpeak\n");
  if(!readText()) {
    return false;
  }
  swLog("Starting speakText: %s\n", textBuffer);
  writeBool(swSpeakText((char *)textBuffer));
  return true;
}

// Execute a speak character command.  This will not return until the character
// has been synthesized, unless processAudio fails to read "true" from the
// client after sending speech samples.
static bool execChar(void) {
  swLog("entering execChar\n");
  validateLine();  // Make sure it is valid UTF-8.
  char *charName = (char *)linePos;
  while(*charName == ' ') {
    charName++;
  }
  bool valid = false;
  uint32_t unicodeChar; 
  uint8_t *lineEnd = line + sizeof(line);
  size_t length = swFindUTF8LengthAndValidate(charName, lineEnd - linePos, &valid, &unicodeChar);
  if (charName[length] != '\0') {
    return false;
  }
  writeBool(valid && swSpeakChar(unicodeChar));
  return true;
}

// Just send a simple summary of the commands.
static void execHelp(void) {
  putClient(
    "cancel     - Interrupt speech while being synthesized\n"
    "quit/exit    - Close the connection and kill the speech server\n"
    "get samplerate - Show the sample rate in Hertz\n"
    "get voices   - List available voices\n"
    "get variants   - List available variations on voices\n"
    "get encoding   - Either UTF-8 or ANSI (most use UTF-8)\n"
    "help       - This command\n"
    "set voice    - Select a voice by it's identifier\n"
    "set variant  - Select a voice variant by it's identifier\n"
    "set pitch    - Set the pitch\n"
    "set punctuation [none|some|most|all] - Set punctuation level\n"
    "set speed    - Set the speed of speech\n"
    "set ssml [true|false] - Enable or disable ssml support\n"
    "speak      - Enter text on separate lines, ending with \".\" on a line by\n"
    "         itself.  Synthesized samples will be generated in hexidecimal\n"
    "char <characther> - Speak a character, encoded in UTF-8.\n"
    "get version  - Report the speech-switch protocol version, currently 1\n"
    "get sonicpitch - Return \"true\" if speech pitch should be adjusted with Sonic.\n"
    "get sonicspeed - Return \"true\" if speech speed should be adjusted with Sonic.\n");
}

// Execute the current command stored in 'line'.  If we read a close command, return false. 
static bool executeCommand(void) {
  char *command, *key;
  swLog("Executing %s\n", line);
  linePos = line;
  command = readWord();
  if(command == NULL) {
    // Just spaces on the line
    return 1;
  }
  if(!strcasecmp(command, "get")) {
    key = readWord();
    if(!strcasecmp(key, "samplerate")) {
      execGetSampleRate();
    } else if(!strcasecmp(key, "voices")) {
      execGetVoices();
    } else if(!strcasecmp(key, "variants")) {
      execGetVoiceVariants();
    } else if(!strcasecmp(key, "version")) {
      putClient("1");
    } else if(!strcasecmp(key, "encoding")) {
      putClient(useANSI? "ANSI" : "UTF-8");
    } else if(!strcasecmp(key, "sonicpitch")) {
      writeBool(swUseSonicPitch());
    } else if(!strcasecmp(key, "sonicspeed")) {
      writeBool(swUseSonicSpeed());
    } else {
      putClient("Unrecognized command");
    }
  } else if(!strcasecmp(command, "set")) {
    key = readWord();
    if(!strcasecmp(key, "voice")) {
      execSetVoice();
    } else if(!strcasecmp(key, "variant")) {
      execSetVoiceVariant();
    } else if(!strcasecmp(key, "pitch")) {
      execSetPitch();
    } else if(!strcasecmp(key, "speed")) {
      execSetSpeed();
    } else if(!strcasecmp(key, "punctuation")) {
      execSetPunctuation();
    } else if(!strcasecmp(key, "ssml")) {
      execSetSsml();
    } else {
      putClient("Unrecognized command");
    }
  } else if(!strcasecmp(command, "speak")) {
    return execSpeak();
  } else if(!strcasecmp(command, "char")) {
    return execChar();
  } else if(!strcasecmp(command, "cancel")) {
    // Nothing required - already finished speech 
  } else if(!strcasecmp(command, "quit") || !strcasecmp(command, "exit")) {
    return false;
  } else if(!strcasecmp(command, "help")) {
    execHelp();
  } else {
    putClient("Unrecognized command");
  }
  return true;
}

// Convert the short data to hex, in big-endian format.
static char *convertToHex(const short *data, int numSamples) {
  int length = numSamples*4 + 1;
  int i, j;
  char *p, value;
  short sample;

  if(length > speechBufferSize) {
    speechBufferSize = length << 1;
    speechBuffer = (uint8_t *)swRealloc(speechBuffer, speechBufferSize, sizeof(char));
  }
  p = (char *)speechBuffer;
  for(i = 0; i < numSamples; i++) {
    sample = data[i];
    for(j = 0; j < 4; j++) {
      value = (sample >> 12) & 0xf;
      *p++ = value <= 9? '0' + value : 'A' + value - 10;
      sample <<= 4;
    }
  }
  *p++ = '\0';
  return (char *)speechBuffer;
}

// Send audio samples in hex to the client.  Return false if the client cancelled. 
bool swProcessAudio(const short *data, int numSamples) {
  char *hexBuf = convertToHex(data, numSamples);
  putClient(hexBuf);
  if(!readLine()) {
    swLog("Unable to read from client\n");
    return false;
  }
  if(strcasecmp((char *)line, "true")) {
    swLog("Cancelled\n");
    return false;
  }
  return true;
}

// Run the speech server.  The only argument will be a directory where the
// engine may find it's speech data.
int main(int argc, char **argv) {
  char *synthDataDir = NULL;
  if(argc == 2) {
    synthDataDir = argv[1];
  } else if(argc != 1) {
    printf("Usage: %s [data_directory]\n", argv[0]);
    return 1;
  }
  swSetLogFileName("/tmp/speechsw_engine.log");
  if(!swInitializeEngine(synthDataDir)) {
    if(argc == 2) {
      printf("Unable to initialize the TTS engine with data directory %s.\n", argv[1]);
    } else {
      printf("Unable to initialize the TTS engine.\n");
    }
    return 1;
  }
  speechBufferSize = 4096;
  speechBuffer = (uint8_t *)swCalloc(speechBufferSize, sizeof(char));
  textBufferSize = 4096;
  textBuffer = (uint8_t *)swCalloc(textBufferSize, sizeof(char));
  while(readLine() && executeCommand());
  swFree(textBuffer);
  swFree(speechBuffer);
  swCloseEngine();
  return 0;
}
