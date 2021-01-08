// SpeechSwitch interface to the espeak TTS engine.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "espeak/speak_lib.h"
#include "engine.h"

#define ESPEAK_BUFLEN 100

static int sampleRate;
static char *currentVoice, *currentVariant;
static char *variantsDir;
static int ssmlFlag;

// Callback for espeak to return synthesized samples.
static int synthCallback(short *data, int numSamples, espeak_EVENT *events) {
  if (numSamples > 0) {
    if (!swProcessAudio(data, numSamples)) {
      return 1; // Abort synthesis
    }
  }
  return 0;
}

// Initialize the engine.
bool swInitializeEngine(const char *synthdataPath) {
  char *variantsDirName = "espeak-data/voices/!v";
  if (synthdataPath == NULL) {
    if (swFileReadable("/usr/share/espeak-data")) {
      synthdataPath = "/usr/share";
    } else {
      synthdataPath = "/usr/lib/x86_64-linux-gnu";
    }
  }
  if (!swFileReadable(synthdataPath)) {
    fprintf(stderr, "Unable to read espeak data from %s\n", synthdataPath);
    return false;
  }
  variantsDir = (char *)swCalloc(strlen(synthdataPath) + strlen(variantsDirName) + 2, sizeof(char));
  strcpy(variantsDir, synthdataPath);
  if (synthdataPath[strlen(synthdataPath)] != '/') {
    strcat(variantsDir, "/");
  }
  strcat(variantsDir, variantsDirName);
  currentVariant = swCopyString("");
  currentVoice = swCopyString("english");
  sampleRate = espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, ESPEAK_BUFLEN, synthdataPath, 0);
  if (sampleRate == -1) {
    return false;
  }
  ssmlFlag = 0;
  espeak_SetSynthCallback(synthCallback);
  return true;
}

// Close the TTS Engine.
bool swCloseEngine(void) {
  swFree(variantsDir);
  swFree(currentVoice);
  swFree(currentVariant);
  return espeak_Terminate() == EE_OK;
}

// Return the sample rate in Hz
uint32_t swGetSampleRate(void) {
  return sampleRate;
}

// Return an array of char pointers representing names of supported voices.
char **swGetVoices(uint32_t *numVoices) {
  char **voices;
  const char *language, *name, *identifier;
  const espeak_VOICE **espeakVoices;
  int i, xVoice;

  espeakVoices = espeak_ListVoices(NULL);
  *numVoices = 0;
  for (i = 0; espeakVoices[i] != NULL; i++) {
    identifier = espeakVoices[i]->identifier;
    if (strncmp(identifier, "mb/", 3)) {
      (*numVoices)++;
    }
  }
  voices = (char **)swCalloc(*numVoices, sizeof(char *));
  xVoice = 0;
  for (i = 0; espeakVoices[i] != NULL; i++) {
    identifier = espeakVoices[i]->identifier;
    if (strncmp(identifier, "mb/", 3)) {
      name = espeakVoices[i]->name;
      language = espeakVoices[i]->languages + 1;
      voices[xVoice] = (char *)swCalloc(strlen(name) + strlen(language) + 2, sizeof(char));
      strcpy(voices[xVoice], name);
      strcat(voices[xVoice], ",");
      strcat(voices[xVoice], language);
      xVoice++;
    }
  }
  return voices;
}

// Let Sonice handle speed.
bool swUseSonicSpeed(void) {
  return false;
}

// Let Sonic handle pitch.
bool swUseSonicPitch(void) {
  return false;
}

// Select a voice.
bool swSetVoice(const char *voice) {
  char *fullVoice;
  bool retVal;

  fullVoice = swCatStrings(voice, currentVariant);
  retVal = espeak_SetVoiceByName(fullVoice) == EE_OK;
  if (retVal) {
    swFree(currentVoice);
    currentVoice = swCopyString(voice);
  }
  swFree(fullVoice);
  return retVal;
}

// Set the speech speed.  Speed is from -100.0 to 100.0, and 0 is the default.
bool swSetSpeed(float speed) {
  int minSpeed = 80; // In words per minute
  int defaultSpeed = 170;
  int maxSpeed = 900;
  int espeakSpeed = swFactorToRange(speed, 1.0f/6.0f, 6.0f, minSpeed, defaultSpeed, maxSpeed);
  fprintf(stderr, "Setting speed to %u\n", espeakSpeed);
  return espeak_SetParameter(espeakRATE, espeakSpeed, 0) == EE_OK;
}

// Set the pitch.  0 means default, -100 is min pitch, and 100 is max pitch.
bool swSetPitch(float pitch) {
  // Translate to espeak's range of 0..100, with 50 as default.
  int espeakPitch = swFactorToRange(pitch, 1.0f/3.0f, 3.0f, 0, 50, 100);
  return espeak_SetParameter(espeakPITCH, espeakPitch, 0) == EE_OK;
}

// Enable or disable SSML support.
bool swSetSSML(bool value) {
  if (value) {
    ssmlFlag = espeakSSML;
  } else {
    ssmlFlag = 0;
  }
  return true;
}

// Speak the text.  Block until finished.
bool swSpeakText(const char *text) {
  return espeak_Synth(text, strlen(text) + 1, 0, POS_CHARACTER, 0,
    espeakCHARS_UTF8 | ssmlFlag, NULL, NULL) == EE_OK;
}

// Speak the character, which is encoded in UTF-8.  Block until finished.
bool swSpeakChar(uint32_t unicodeChar) {
  return espeak_Char(unicodeChar) == EE_OK;
}

// List voice variants.  This is for formant synths, and is typically stuff
// like MALE2, or CHILD1, though it can be anything.
char **swGetVoiceVariants(uint32_t *numVariants) {
  return swListDirectory(variantsDir, numVariants);
}

// Select a voice variant.
bool swSetVoiceVariant(const char *variant) {
  swFree(currentVariant);
  if (strlen(variant) != 0) {
    currentVariant = swCatStrings("+", variant);
  } else {
    currentVariant = swCopyString("");
  }
  return swSetVoice(currentVoice);
}
