// This simple interface to espeak is designed to be a minimal interface
// demonstraiting how easy it is to add support for a new TTS engine.

#include <string.h>
#include <stdlib.h>
#include "espeak/speak_lib.h"
#include "engine.h"

#define ESPEAK_BUFLEN 100

static int sampleRate;

// Callback for espeak to return synthesized samples.
static int synthCallback(short *data, int numSamples, espeak_EVENT *events)
{
    if(numSamples > 0) {
        if(!swProcessAudio(data, numSamples)) {
            return 1; // Abort synthesis
        }
    }
    return 0;
}

// Initialize the engine.
bool swInitializeEngine(const char *synthdataPath)
{
    if(synthdataPath == NULL) {
        if(swFileReadable("/usr/share/espeak-data")) {
            synthdataPath = "/usr/share";
        } else {
            synthdataPath = "/usr/lib/x86_64-linux-gnu";
        }
    }
    if(!swFileReadable(synthdataPath)) {
        fprintf(stderr, "Unable to read espeak data from %s\n", synthdataPath);
        return false;
    }
    sampleRate = espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, ESPEAK_BUFLEN, synthdataPath, 0);
    if(sampleRate == -1) {
        return false;
    }
    espeak_SetSynthCallback(synthCallback);
    return true;
}

// Close the TTS Engine.
bool swCloseEngine(void)
{
    return espeak_Terminate() == EE_OK;
}

// Return the sample rate in Hz
uint32_t swGetSampleRate(void) {
    return sampleRate;
}

// Return an array of char pointers representing names of supported voices.
char **swGetVoices(uint32_t *numVoices)
{
    char **voices;
    const char *language, *name;
    const espeak_VOICE **espeakVoices;
    int i;

    espeakVoices = espeak_ListVoices(NULL);
    *numVoices = 0;
    for(i = 0; espeakVoices[i] != NULL; i++) {
        (*numVoices)++;
    }
    voices = (char **)calloc(*numVoices, sizeof(char *));
    for(i = 0; i < *numVoices; i++) {
        name = espeakVoices[i]->name;
        language = espeakVoices[i]->languages + 1;
        voices[i] = (char *)calloc(strlen(name) + strlen(language) + 2, sizeof(char));
        strcpy(voices[i], name);
        strcat(voices[i], ",");
        strcat(voices[i], language);
    }
    return voices;
}

// Select a voice.
bool swSetVoice(const char *voice)
{
    return espeak_SetVoiceByName(voice) == EE_OK;
}

// Assume SpeechHub will do this for us.
bool swSetSpeed(float speed)
{
    return false;
}

// Assume SpeechHub will do this for us.
bool swSetPitch(float pitch)
{
    return false;
}

// Assume SpeechHub will do this for us.
bool swSetPunctuationLevel(int level)
{
    return false;
}

// Enable or disable SSML support.
bool swSetSSML(bool value)
{
    return false;
}

// Speak the text.  Block until finished.
bool swSpeakText(const char *text)
{
    return espeak_Synth(text, strlen(text) + 1, 0, POS_CHARACTER, 0,
        espeakCHARS_UTF8, NULL, NULL) == EE_OK;
}

// Don't support variants.
char **swGetVoiceVariants(uint32_t *numVariants)
{
    return NULL;
}

// Dont support variants.
bool swSetVoiceVariant(const char *variant)
{
    return false;
}
