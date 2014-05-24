// Speech-Hub interface to the espeak TTS engine.

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
static int synthCallback(short *data, int numSamples, espeak_EVENT *events)
{
    if(numSamples > 0) {
        if(!processAudio(data, numSamples)) {
            return 1; // Abort synthesis
        }
    }
    return 0;
}

// Initialize the engine.
bool initializeEngine(char *synthdataPath)
{
    char *variantsDirName = "espeak-data/voices/!v";
    if(synthdataPath == NULL) {
        synthdataPath = "/usr/share/";
    }
    variantsDir = (char *)calloc(strlen(synthdataPath) + strlen(variantsDirName) + 2, sizeof(char));
    strcpy(variantsDir, synthdataPath);
    if(synthdataPath[strlen(synthdataPath)] != '/') {
        strcat(variantsDir, "/");
    }
    strcat(variantsDir, variantsDirName);
    currentVariant = copyString("");
    currentVoice = copyString("english");
    sampleRate = espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, ESPEAK_BUFLEN, synthdataPath, 0);
    if(sampleRate == -1) {
        return false;
    }
    ssmlFlag = 0;
    espeak_SetSynthCallback(synthCallback);
    return true;
}

// Close the TTS Engine.
bool closeEngine(void)
{
    free(variantsDir);
    free(currentVoice);
    free(currentVariant);
    return espeak_Terminate() == EE_OK;
}

// Return the sample rate in Hz
int getSampleRate(void) {
    return sampleRate;
}

// Return an array of char pointers representing names of supported voices.
char **getVoices(int *numVoices)
{
    char **voices;
    const char *language, *name, *identifier;
    const espeak_VOICE **espeakVoices;
    int i, xVoice;

    espeakVoices = espeak_ListVoices(NULL);
    *numVoices = 0;
    for(i = 0; espeakVoices[i] != NULL; i++) {
        identifier = espeakVoices[i]->identifier;
        if(strncmp(identifier, "mb/", 3)) {
            (*numVoices)++;
        }
    }
    voices = (char **)calloc(*numVoices, sizeof(char *));
    xVoice = 0;
    for(i = 0; espeakVoices[i] != NULL; i++) {
        identifier = espeakVoices[i]->identifier;
        if(strncmp(identifier, "mb/", 3)) {
            name = espeakVoices[i]->name;
            language = espeakVoices[i]->languages + 1;
            voices[xVoice] = (char *)calloc(strlen(name) + strlen(language) + 2, sizeof(char));
            strcpy(voices[xVoice], name);
            strcat(voices[xVoice], ",");
            strcat(voices[xVoice], language);
            xVoice++;
        }
    }
    return voices;
}

// Select a voice.
bool setVoice(char *voice)
{
    char *fullVoice;
    bool retVal;

    fullVoice = catStrings(voice, currentVariant);
    retVal = espeak_SetVoiceByName(fullVoice) == EE_OK;
    if(retVal) {
        free(currentVoice);
        currentVoice = copyString(voice);
    }
    free(fullVoice);
    return retVal;
}

// Set the speech speed.  Speed is from -100.0 to 100.0, and 0 is the default.
bool setSpeed(float speed)
{
    int espeakSpeed;
    float minSpeed = 80.0f; // In words per minute
    float defaultSpeed = 170.0f;
    float maxSpeed = 800.0f;

    if(speed >= 0.0f) {
        espeakSpeed = (int)((defaultSpeed + speed*(maxSpeed - defaultSpeed)/100.0f) + 0.5f);
    } else {
        espeakSpeed = (int)((defaultSpeed + speed*(defaultSpeed - minSpeed)/100.0f) + 0.5f);
    }
    return espeak_SetParameter(espeakRATE, espeakSpeed, 0) == EE_OK;
}

// Set the pitch.  0 means default, -100 is min pitch, and 100 is max pitch.
bool setPitch(float pitch)
{
    // Translate to espeak's range of 0..100, with 50 as default.
    int espeakPitch = (int)((pitch + 100.0f)/2.0f + 0.5f);

    return espeak_SetParameter(espeakPITCH, espeakPitch, 0) == EE_OK;
}

// Set the punctuation leve, which will be PUNCT_NONE, PUNCT_SOME, or PUNCT_ALL.
bool setPunctuationLevel(int level)
{
    int espeakLevel;

    switch(level) {
    case PUNCT_NONE:
        espeakLevel = espeakPUNCT_NONE;
        break;
    case PUNCT_SOME:
    case PUNCT_MOST:
        espeakLevel = espeakPUNCT_SOME;
        break;
    case PUNCT_ALL:
        espeakLevel = espeakPUNCT_ALL;
    default:
        return false;
    }
    return espeak_SetParameter(espeakPUNCTUATION, espeakLevel, 0) == EE_OK;
}

// Enable or disable SSML support.
bool setSSML(bool value)
{
    if(value) {
        ssmlFlag = espeakSSML;
    } else {
        ssmlFlag = 0;
    }
    return true;
}

// Speak the text.  Block until finished.
bool speakText(char *text)
{
    return espeak_Synth(text, strlen(text) + 1, 0, POS_CHARACTER, 0,
        espeakCHARS_UTF8 | ssmlFlag, NULL, NULL) == EE_OK;
}

// List voice variants.  This is for formant synths, and is typically stuff
// like MALE2, or CHILD1, though it can be anything.
char **getVoiceVariants(int *numVariants)
{
    return listDirectory(variantsDir, numVariants);
}

// Select a voice variant.
bool setVoiceVariant(char *variant)
{
    free(currentVariant);
    if(strlen(variant) != 0) {
        currentVariant = catStrings("+", variant);
    } else {
        currentVariant = copyString("");
    }
    return setVoice(currentVoice);
}
