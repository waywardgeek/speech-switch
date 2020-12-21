// SpeechSwitch interface to the IBM TTS engine.

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <eci.h>
#include <unistd.h>
#include "engine.h"

#define IBMTTS_BUFLEN 1024

// Globals are declared with sw prefix.
static short swBuffer[IBMTTS_BUFLEN];
static ECIHand swEciHandle;
static int swDefaultSpeed, swDefaultPitch;
static float swCurrentSpeed = 0.0f, swCurrentPitch = 0.0f;
static bool swCancelled;

// This structure was copied from speech-dispatcher's ibmtts.c
typedef struct {
  char *name;
  char *lang;
  char *dialect;
  enum ECILanguageDialect langID;
  char* charset;
} eciLocale;

// This one, too.
typedef enum {
  eciTextModeDefault = 0,
  eciTextModeAlphaSpell = 1,
  eciTextModeAllSpell = 2,
  eciIRCSpell = 3
} ECITextMode;

static const eciLocale swEciLocales[] = {
  {"American_English", "en", "US", eciGeneralAmericanEnglish, "ISO-8859-1"},
  {"British_English", "en", "GB", eciBritishEnglish, "ISO-8859-1"},
  {"Castilian_Spanish", "es", "ES", eciCastilianSpanish, "ISO-8859-1"},
  {"Mexican_Spanish", "es", "MX", eciMexicanSpanish, "ISO-8859-1"},
  {"French", "fr", "FR", eciStandardFrench, "ISO-8859-1"},
  {"Canadian_French", "ca", "FR", eciCanadianFrench, "ISO-8859-1"},
  {"German", "de", "DE", eciStandardGerman, "ISO-8859-1"},
  {"Italian", "it", "IT", eciStandardItalian, "ISO-8859-1"},
  {"Mandarin_Chinese", "zh", "CN", eciMandarinChinese, "GBK"},
  {"Mandarin_Chinese GB", "zh", "CN_GB", eciMandarinChineseGB, "GBK"},
  {"Mandarin_Chinese PinYin", "zh", "CN_PinYin", eciMandarinChinesePinYin, "GBK"},
  {"Mandarin_Chinese UCS", "zh", "CN_UCS", eciMandarinChineseUCS, "UCS2"},
  {"Taiwanese_Mandarin", "zh", "TW", eciTaiwaneseMandarin, "BIG5"},
  {"Taiwanese_Mandarin Big 5", "zh", "TW_Big5", eciTaiwaneseMandarinBig5, "BIG5"},
  {"Taiwanese_Mandarin ZhuYin", "zh", "TW_ZhuYin", eciTaiwaneseMandarinZhuYin, "BIG5"},
  {"Taiwanese_Mandarin PinYin", "zh", "TW_PinYin", eciTaiwaneseMandarinPinYin, "BIG5"},
  {"Taiwanese_Mandarin UCS", "zh", "TW_UCS", eciTaiwaneseMandarinUCS, "UCS2"},
  {"Brazilian_Portuguese", "pt", "BR", eciBrazilianPortuguese, "ISO-8859-1"},
  {"Japanese", "ja", "JP", eciStandardJapanese, "SJIS"},
  {"Japanese_SJIS", "ja", "JP_SJIS", eciStandardJapaneseSJIS, "SJIS"},
  {"Japanese_UCS", "ja", "JP_UCS", eciStandardJapaneseUCS, "UCS2"},
  {"Finnish", "fi", "FI", eciStandardFinnish, "ISO-8859-1"},
  {"Korean", "ko", "KR", eciStandardKorean, "UHC"},
  {"Korean_UHC", "ko", "KR_UHC", eciStandardKoreanUHC, "UHC"},
  {"Korean_UCS", "ko", "KR_UCS", eciStandardKoreanUCS, "UCS2"},
  {"Cantonese", "zh", "HK", eciStandardCantonese, "GBK"},
  {"Cantonese_GB", "zh", "HK_GB", eciStandardCantoneseGB, "GBK"},
  {"Cantonese_UCS", "zh", "HK_UCS", eciStandardCantoneseUCS, "UCS2"},
  {"HongKong_Cantonese", "zh", "HK", eciHongKongCantonese, "BIG5"},
  {"HongKong_Cantonese Big 5", "zh", "HK_BIG5", eciHongKongCantoneseBig5, "BIG5"},
  {"HongKong_Cantonese UCS", "zh", "HK_UCS", eciHongKongCantoneseUCS, "UCS-2"},
  {"Dutch", "nl", "BE", eciStandardDutch, "ISO-8859-1"},
  {"Norwegian", "no", "NO", eciStandardNorwegian, "ISO-8859-1"},
  {"Swedish", "sv", "SE", eciStandardSwedish, "ISO-8859-1"},
  {"Danish", "da", "DK", eciStandardDanish, "ISO-8859-1"},
  {"Reserved", "en", "US", eciStandardReserved, "ISO-8859-1"},
  {"Thai", "th", "TH", eciStandardThai, "TIS-620"},
  {"ThaiTIS", "th", "TH_TIS", eciStandardThaiTIS, "TIS-620"},
  {NULL, 0, NULL}
};

const char *swVariants[13] = {
  "Default",
  "Male1",
  "Female1",
  "Child1",
  "Male2",
  "Male3",
  "Female2",
  "ElderlyFemale",
  "ElderlyMale",
  "Unknown1", // Future-proof a bit by allowing for new variants
  "Unknown2",
  "Unknown3",
  "Unknown4"};

#define NUM_LANGUAGES (sizeof(swEciLocales)/sizeof(swEciLocales[0]) - 1)

// Print an error message.
static void error(void) {
  char buf[200]; // Docs says it must be at least 100 long
  eciErrorMessage(swEciHandle, buf);
  printf("Error: %s\n", buf);
}

// Process data from the synth engine.  Just forward it to the audio processor.
static enum ECICallbackReturn synthCallback(ECIHand eciHandle, enum ECIMessage msg,
    long lparam, void* data) {
  if (msg == eciWaveformBuffer) {
    if (!swCancelled && !swProcessAudio(swBuffer, lparam)) {
      swCancelled = true; // eciStop does not seem to work when called from Windows, even on a separate thread.
      return eciDataAbort; // The docs say this cancels synthesis, but it doesn't
    }
  }
  return eciDataProcessed;
}

// Read the default speed and pitch parameters.
static void setDefaultPitchAndSpeed(void) {
  swDefaultSpeed = eciGetVoiceParam(swEciHandle, 0, eciSpeed);
  swDefaultPitch = eciGetVoiceParam(swEciHandle, 0, eciPitchBaseline);
}

// Initialize the engine.
bool swInitializeEngine(const char *synthdataPath) {
  // ibmtts Only supports ANSI.
  swSwitchToANSI();
  swCancelled = false;
  swEciHandle = eciNew();
  if (swEciHandle == NULL_ECI_HAND) {
    return false;
  }
  eciRegisterCallback(swEciHandle, synthCallback, NULL);
  eciSetOutputBuffer(swEciHandle, IBMTTS_BUFLEN, swBuffer);
  setDefaultPitchAndSpeed();
  return true;
}

// Close the TTS Engine.
bool swCloseEngine(void) {
  eciDelete(swEciHandle);
  return true;
}

// Return the sample rate.
uint32_t swGetSampleRate(void) {
  switch(eciGetParam(swEciHandle, eciSampleRate)) {
  case 0: return 8000;
  case 1: return 11025;
  case 2: return 22050;
  }
  return 0; // Dummy return;
}

// Look through the table of locales to find the matching language entry.
static eciLocale findLocaleFromID(enum ECILanguageDialect langID) {
  int i;
  for (i = 0; i < NUM_LANGUAGES; i++) {
    if (swEciLocales[i].langID == langID) {
      return swEciLocales[i];
    }
  }
  return swEciLocales[0]; // Default to English
}

// Return an array of char pointers representing names of supported voices.
char **swGetVoices(uint32_t *numVoices) {
  enum ECILanguageDialect language[NUM_LANGUAGES];
  eciLocale locale;
  char **voices, *p;
  uint32_t i;
  // For some reason, the first call fails in MSVC
  *numVoices = NUM_LANGUAGES;
  eciGetAvailableLanguages(NULL, (int *)numVoices);
  if (eciGetAvailableLanguages(language, (int *)numVoices) != 0) {
    error();
    *numVoices = 0;
    return NULL;
  }
  voices = (char **)swCalloc(*numVoices, sizeof(char *));
  for (i = 0; i < *numVoices; i++) {
    locale = findLocaleFromID(language[i]);
    voices[i] = (char *)swCalloc(strlen(locale.name) + strlen(locale.lang) + strlen(locale.dialect) + 3,
      sizeof(char));
    strcpy(voices[i], locale.name);
    strcat(voices[i], ",");
    p = voices[i] + strlen(voices[i]);
    strcpy(p, locale.lang);
    strcat(p, "-");
    strcat(p, locale.dialect);
    while(*p) {
      *p = tolower(*p);
      p++;
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

// Find a language from it's name.
static eciLocale findLocaleFromName(const char *name, bool *foundIt) {
  int i;
  for (i = 0; i < NUM_LANGUAGES; i++) {
    if (!strcasecmp(swEciLocales[i].name, name)) {
      *foundIt = true;
      return swEciLocales[i];
    }
  }
  *foundIt = false;
  return swEciLocales[0];
}

// Select a voice.
bool swSetVoice(const char *voice) {
  bool foundIt;
  eciLocale locale = findLocaleFromName(voice, &foundIt);
  if (!foundIt) {
    return false;
  }
  eciSetParam(swEciHandle, eciLanguageDialect, locale.langID);
  return true;
}

// Set the speech speed.  Speed is from 1/6X to 6X, and 1.0 is the default.
bool swSetSpeed(float speed) {
  // speed is between 1/6 and 6.  Integer range is 0 to 250, but speed is
  // normally limited to 150 or so.
  swCurrentSpeed = swFactorToRange(speed, 1.0f/6.0f, 6.0f, 0, swDefaultSpeed, 170);
  return eciSetVoiceParam(swEciHandle, 0, eciSpeed, swCurrentSpeed) != -1;
}

// Set the pitch.  0 means default, -100 is min pitch, and 100 is max pitch.
bool swSetPitch(float pitch) {
  // pitch is between 1/3 and 3.
  swCurrentPitch = swFactorToRange(pitch, 1.0f/3.0f, 3.0f, 0, swDefaultPitch, 100);
  return eciSetVoiceParam(swEciHandle, 0, eciPitchBaseline, swCurrentPitch) != -1;
}

// Set the punctuation level, which will be PUNCT_NONE, PUNCT_SOME, PUNCT_MOST, or PUNCT_ALL.
bool swSetPunctuationLevel(swPunctLevel level) {
  return true; // No support for punctuation levels
}

// Enable or disable SSML support.
bool swSetSSML(bool value) {
  return true; // No support for SSML
}

// Speak the text.  For some reason, we have to call etiSynchronize here, making
// it impossible to process audio output while synthesizing in small blocks, or
// to abort.
bool swSpeakText(const char *text) {
  swCancelled = false;
  eciAddText(swEciHandle, text);
  eciSynthesize(swEciHandle);
  eciSynchronize(swEciHandle);
  return true;
}

// Speak the character, which is encoded in UTF-8.  Block until finished.
bool swSpeakChar(uint32_t unicodeChar) {
  // Only can speak ANSI characters.
  uint8_t ansiChar = swUnicodeToAnsi(unicodeChar);
  if (ansiChar == '\0') {
    return false;
  }
  char charText[] = {(char)ansiChar, '\0'};
  eciSetParam(swEciHandle, eciTextMode, eciTextModeAllSpell);
  swCancelled = false;
  eciAddText(swEciHandle, charText);
  eciSynthesize(swEciHandle);
  eciSynchronize(swEciHandle);
  eciSetParam(swEciHandle, eciTextMode, eciTextModeDefault);
  return true;
}

// List voice variants.  This is for formant synths, and is typically stuff
// like MALE2, or CHILD1, though it can be anything.
char **swGetVoiceVariants(uint32_t *numVariants) {
  *numVariants = ECI_PRESET_VOICES + 1;
  return swCopyStringList(swVariants, ECI_PRESET_VOICES + 1);
}

// Select a voice variant.
bool swSetVoiceVariant(const char *variant) {
  int i;
  bool result;
  for (i = 0; i < ECI_PRESET_VOICES + 1; i++) {
    if (!strcasecmp(variant, swVariants[i])) {
      if (i == 0) {
        result = eciCopyVoice(swEciHandle, 1, 0); // Set default voice, which is 1
      } else {
        result = eciCopyVoice(swEciHandle, i, 0);
      }
      if (result) {
        // Ibmtts resets speed and pitch to default when setting a voice variant.
        setDefaultPitchAndSpeed();
        swSetPitch(swCurrentPitch);
        swSetSpeed(swCurrentSpeed);
      }
      return result;
    }
  }
  return false;
}
