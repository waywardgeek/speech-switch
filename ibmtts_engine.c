// Speech-Hub interface to the IBM TTS engine.

#include <stdio.h> // Only for debugging
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <eci.h>
#include <unistd.h>
#include "engine.h"

#define IBMTTS_BUFLEN 1024

static short buffer[IBMTTS_BUFLEN];
static ECIHand eciHandle;
static int defaultSpeed, defaultPitch;
static float currentSpeed = 0.0f, currentPitch = 0.0f;
static bool cancelled;

// This structure was copied from speech-dispatcher's ibmtts.c
typedef struct _eciLocale {
    char *name;
    char *lang;
    char *dialect;
    enum ECILanguageDialect langID;
    char* charset;
} eciLocale;

static eciLocale eciLocales[] = {
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

char *variants[13] = {
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

#define NUM_LANGUAGES (sizeof(eciLocales)/sizeof(eciLocales[0]) - 1)

// Print an error message.
static void error(void)
{
    char buf[200]; // Docs says it must be at least 100 long

    eciErrorMessage(eciHandle, buf);
    printf("Error: %s\n", buf);
}

// Process data from the synth engine.  Just forward it to the audio processor.
static enum ECICallbackReturn synthCallback(ECIHand eciHandle, enum ECIMessage msg,
    long lparam, void* data)
{
    if(msg == eciWaveformBuffer) {
        if(!cancelled && !swProcessAudio(buffer, lparam)) {
            cancelled = true; // eciStop does not seem to work when called from Windows, even on a separate thread.
            return eciDataAbort; // The docs say this cancels synthesis, but it doesn't
        }
    }
    return eciDataProcessed;
}

// Read the default speed and pitch parameters.
static void setDefaultPitchAndSpeed(void)
{
    defaultSpeed = eciGetVoiceParam(eciHandle, 0, eciSpeed);
    defaultPitch = eciGetVoiceParam(eciHandle, 0, eciPitchBaseline);
}

// Initialize the engine.
bool swInitializeEngine(char *synthdataPath)
{
    // ibmtts Only supports ANSI.
    swSwitchToANSI();
    cancelled = false;
    eciHandle = eciNew();
    if(eciHandle == NULL_ECI_HAND) {
        return false;
    }
    eciRegisterCallback(eciHandle, synthCallback, NULL);
    eciSetOutputBuffer(eciHandle, IBMTTS_BUFLEN, buffer);
    setDefaultPitchAndSpeed();
    return true;
}

// Close the TTS Engine.
bool swCloseEngine(void)
{
    eciDelete(eciHandle);
    return true;
}

// Return the sample rate.
uint32_t swGetSampleRate(void)
{
    switch(eciGetParam(eciHandle, eciSampleRate)) {
    case 0: return 8000;
    case 1: return 11025;
    case 2: return 22050;
    }
    return 0; // Dummy return;
}

// Look through the table of locales to find the matching language entry.
static eciLocale findLocaleFromID(enum ECILanguageDialect langID)
{
    int i;

    for(i = 0; i < NUM_LANGUAGES; i++) {
        if(eciLocales[i].langID == langID) {
            return eciLocales[i];
        }
    }
    return eciLocales[0]; // Default to English
}

// Return an array of char pointers representing names of supported voices.
char **swGetVoices(uint32_t *numVoices)
{
    enum ECILanguageDialect language[NUM_LANGUAGES];
    eciLocale locale;
    char **voices, *p;
    uint32_t i;

    // For some reason, the first call fails in MSVC
    *numVoices = NUM_LANGUAGES;
    eciGetAvailableLanguages(NULL, (int *)numVoices);
    if(eciGetAvailableLanguages(language, (int *)numVoices) != 0) {
        error();
        *numVoices = 0;
        return NULL;
    }
    voices = (char **)calloc(*numVoices, sizeof(char *));
    for(i = 0; i < *numVoices; i++) {
        locale = findLocaleFromID(language[i]);
        voices[i] = (char *)calloc(strlen(locale.name) + strlen(locale.lang) + strlen(locale.dialect) + 3,
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

// Find a language from it's name.
static eciLocale findLocaleFromName(char *name)
{
    int i;

    for(i = 0; i < NUM_LANGUAGES; i++) {
        if(!strcasecmp(eciLocales[i].name, name)) {
            return eciLocales[i];
        }
    }
    return eciLocales[0]; // Dummy return
}

// Select a voice.
bool swSetVoice(char *voice)
{
    eciLocale locale = findLocaleFromName(voice);

    return eciSetParam(eciHandle, eciLanguageDialect, locale.langID) != -1;
}

// Set the speech speed.  Speed is from -100.0 to 100.0, and 0 is the default.
bool swSetSpeed(float speed)
{
    int ibmttsSpeed;
    float minSpeed = 0.0f; // In words per minute
    float maxSpeed = 150.0f;

    if(speed >= 0.0f) {
        ibmttsSpeed = (int)((defaultSpeed + speed*(maxSpeed - defaultSpeed)/100.0f) + 0.5f);
    } else {
        ibmttsSpeed = (int)((defaultSpeed + speed*(defaultSpeed - minSpeed)/100.0f) + 0.5f);
    }
    currentSpeed = speed;
    return eciSetVoiceParam(eciHandle, 0, eciSpeed, ibmttsSpeed) != -1;
}

// Set the pitch.  0 means default, -100 is min pitch, and 100 is max pitch.
bool swSetPitch(float pitch)
{
    float minPitch = 0.0f;
    float maxPitch = 100.0f;
    int newPitch;

    if(pitch >= 0.0f) {
        newPitch = (int)((defaultPitch + pitch*(maxPitch - defaultPitch)/100.0f) + 0.5f);
    } else {
        newPitch = (int)((defaultPitch + pitch*(defaultPitch - minPitch)/100.0f) + 0.5f);
    }
    currentPitch = pitch;
    return eciSetVoiceParam(eciHandle, 0, eciPitchBaseline, newPitch) != -1;
}

// Set the punctuation level, which will be PUNCT_NONE, PUNCT_SOME, PUNCT_MOST, or PUNCT_ALL.
bool swSetPunctuationLevel(int level)
{
    return true; // No support for punctuation levels
}

// Enable or disable SSML support.
bool swSetSSML(bool value)
{
    return true; // No support for SSML
}

// Speak the text.  For some reason, we have to call etiSynchronize here, making
// it impossible to process audio output while synthesizing in small blocks, or
// to abort.
bool swSpeakText(char *text)
{
    cancelled = false;
    eciAddText(eciHandle, text);
    eciSynthesize(eciHandle);
    eciSynchronize(eciHandle);
    return true;
}

// List voice variants.  This is for formant synths, and is typically stuff
// like MALE2, or CHILD1, though it can be anything.
char **swGetVoiceVariants(uint32_t *numVariants)
{
    *numVariants = ECI_PRESET_VOICES + 1;
    return swCopyStringList(variants, ECI_PRESET_VOICES + 1);
}

// Select a voice variant.
bool swSetVoiceVariant(char *variant)
{
    int i;
    bool result;

    for(i = 0; i < ECI_PRESET_VOICES + 1; i++) {
        if(!strcasecmp(variant, variants[i])) {
            if(i == 0) {
                result = eciCopyVoice(eciHandle, 1, 0); // Set default voice, which is 1
            } else {
                result = eciCopyVoice(eciHandle, i, 0);
            }
            if(result) {
                // Ibmtts resets speed and pitch to default when setting a voice variant.
                setDefaultPitchAndSpeed();
                swSetPitch(currentPitch);
                swSetSpeed(currentSpeed);
            }
            return result;
        }
    }
    return false;
}
