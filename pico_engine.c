// Speech-Hub interface to the svox pico TTS engine.

// This was written in 2011 by Bill Cox, with much code copied directly from
// pico2wave.c from the Debian svox package.  I claim no additional copyright to
// this work, and place my contributions in the public domain.  The copyright
// from pico2wave.c is:

/* pico2wave.c

 * Copyright (C) 2009 Mathieu Parent <math.parent@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *   Convert text to .wav using svox text-to-speech system.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <picoapi.h>
#include <picoapid.h>
#include <picoos.h>
#include "engine.h"

/* adaptation layer defines */
#define PICO_MEM_SIZE       2500000
#define DummyLen 100000000

/* string constants */
#define MAX_OUTBUF_SIZE     512
#ifdef picolangdir
static char * PICO_LINGWARE_PATH             = picolangdir "/";
#else
static char * PICO_LINGWARE_PATH             = "/usr/share/pico/lang";
#endif
static char * PICO_VOICE_NAME                = "PicoVoice";

/* supported voices
   Pico does not seperately specify the voice and locale.   */
static char *picoLanguageNames[] = {"US English", "UK English", "German", "Spanish", "French", "Italian"};
static char *picoSupportedLang[] = {"en-us", "en-gb", "de-de", "es-es", "fr-fr", "it-it"};
static char *picoInternalTaLingware[] = {"en-US_ta.bin", "en-GB_ta.bin", "de-DE_ta.bin", "es-ES_ta.bin", "fr-FR_ta.bin", "it-IT_ta.bin"};
static char *picoInternalSgLingware[] = {"en-US_lh0_sg.bin", "en-GB_kh0_sg.bin", "de-DE_gl0_sg.bin", "es-ES_zl0_sg.bin", "fr-FR_nk0_sg.bin", "it-IT_cm0_sg.bin"};
static int picoNumSupportedVoices = 6;

/* adapation layer global variables */
static void *picoMemArea = NULL;
static pico_System picoSystem = NULL;
static pico_Resource picoTaResource = NULL;
static pico_Resource picoSgResource = NULL;
static pico_Resource picoUtppResource = NULL;
static pico_Engine picoEngine = NULL;
static char *picoTaFileName = NULL;
static char *picoSgFileName = NULL;
static char *picoTaResourceName = NULL;
static char *picoSgResourceName = NULL;
static char *picoSynthdataPath = NULL;

// Find the language index of the named voice.  If it does not exist, the first
// language.
static int findLanguageIndex(char *name)
{
    int i;

    for(i = 0; i < picoNumSupportedVoices; i++) {
        if(!strcmp(name, picoLanguageNames[i])) {
            return i;
        }
    }
    return 0;
}

// Load a voice
static bool loadVoice(char *name)
{
    int langIndex = findLanguageIndex(name);
    pico_Retstring outMessage;
    char *fileName;
    int ret;

    /* Load the text analysis Lingware resource file. */
    fileName = picoInternalTaLingware[langIndex];
    picoTaFileName = (char *)calloc(strlen(picoSynthdataPath) + strlen(fileName) + 1, sizeof(char));
    strcpy((char *)picoTaFileName, picoSynthdataPath);
    strcat((char *)picoTaFileName, fileName);
    ret = pico_loadResource(picoSystem, (pico_Char *)picoTaFileName, &picoTaResource);
    if(ret) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot load text analysis resource file (%i): %s\n", ret, outMessage);
        return false;
    }
    /* Load the signal generation Lingware resource file. */
    fileName = picoInternalSgLingware[langIndex];
    picoSgFileName = (char *)calloc(strlen(picoSynthdataPath) + strlen(fileName) + 1, sizeof(char));
    strcpy((char *)picoSgFileName, picoSynthdataPath);
    strcat((char *)picoSgFileName, fileName);
    ret = pico_loadResource(picoSystem, (pico_Char *)picoSgFileName, &picoSgResource);
    if(ret) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot load signal generation Lingware resource file (%i): %s\n", ret, outMessage);
        return false;
    }
    /* Get the text analysis resource name. */
    picoTaResourceName = (char *)calloc(PICO_MAX_RESOURCE_NAME_SIZE, sizeof(char));
    ret = pico_getResourceName(picoSystem, picoTaResource, (char *)picoTaResourceName);
    if(ret) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot get the text analysis resource name (%i): %s\n", ret, outMessage);
        return false;
    }
    /* Get the signal generation resource name. */
    picoSgResourceName = (char *)calloc(PICO_MAX_RESOURCE_NAME_SIZE, sizeof(char));
    ret = pico_getResourceName(picoSystem, picoSgResource, (char *)picoSgResourceName);
    if(ret) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot get the signal generation resource name (%i): %s\n", ret, outMessage);
        return false;
    }
    /* Create a voice definition.   */
    ret = pico_createVoiceDefinition(picoSystem, (pico_Char *)PICO_VOICE_NAME);
    if(ret) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot create voice definition (%i): %s\n", ret, outMessage);
        return false;
    }
    /* Add the text analysis resource to the voice. */
    ret = pico_addResourceToVoiceDefinition(picoSystem, (pico_Char *)PICO_VOICE_NAME, (pico_Char *)picoTaResourceName);
    if(ret) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot add the text analysis resource to the voice (%i): %s\n", ret, outMessage);
        return false;
    }
    /* Add the signal generation resource to the voice. */
    ret = pico_addResourceToVoiceDefinition(picoSystem, (pico_Char *)PICO_VOICE_NAME, (pico_Char *)picoSgResourceName);
    if(ret) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot add the signal generation resource to the voice (%i): %s\n", ret, outMessage);
        return false;
    }
    /* Create a new Pico engine. */
    ret = pico_newEngine(picoSystem, (pico_Char *)PICO_VOICE_NAME, &picoEngine);
    if(ret) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot create a new pico engine (%i): %s\n", ret, outMessage);
        return false;
    }
    return true;
}

// Unload a voice.
static void unloadVoice(void)
{
    if(picoEngine) {
        pico_disposeEngine(picoSystem, &picoEngine);
        pico_releaseVoiceDefinition(picoSystem, (pico_Char *)PICO_VOICE_NAME);
        picoEngine = NULL;
    }
    if(picoUtppResource) {
        pico_unloadResource(picoSystem, &picoUtppResource);
        picoUtppResource = NULL;
    }
    if(picoSgResource) {
        pico_unloadResource(picoSystem, &picoSgResource);
        picoSgResource = NULL;
    }
    if(picoTaResource) {
        pico_unloadResource(picoSystem, &picoTaResource);
        picoTaResource = NULL;
    }
}

// Initialize the engine.
bool initializeEngine(char *synthdataPath)
{
    pico_Retstring outMessage;
    int ret;
    int length;

    if(synthdataPath == NULL) {
        synthdataPath = PICO_LINGWARE_PATH;
    }
    length = strlen(synthdataPath);
    picoSynthdataPath = (char *)calloc(length + 2, sizeof(char));
    strcpy(picoSynthdataPath, synthdataPath);
    if(length == 0 || synthdataPath[length - 1] != '/') {
        strcpy(picoSynthdataPath + length, "/");
    }
    picoMemArea = malloc(PICO_MEM_SIZE);
    ret = pico_initialize(picoMemArea, PICO_MEM_SIZE, &picoSystem);
    if(ret) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot initialize pico (%i): %s\n", ret, outMessage);
        return false;
    }
    return loadVoice("US English");
}

// Close the TTS Engine.
bool closeEngine(void)
{
    unloadVoice();
    if(picoSystem) {
        pico_terminate(&picoSystem);
        picoSystem = NULL;
    }
    return true;
}

// Return the sample rate in Hz
int getSampleRate(void) {
    return 16000;
}

// Return an array of char pointers representing names of supported voices.
char **getVoices(int *numVoices)
{
    char **voices;
    char *name, *language;
    int i;

    *numVoices = picoNumSupportedVoices;
    voices = (char **)calloc(picoNumSupportedVoices, sizeof(char *));
    for(i = 0; i <picoNumSupportedVoices; i++) {
        name = picoLanguageNames[i];
        language = picoSupportedLang[i];
        voices[i] = (char *)calloc(strlen(name) + strlen(language) + 2, sizeof(char));
        strcpy(voices[i], name);
        strcat(voices[i], ",");
        strcat(voices[i], language);
    }
    return voices;
}

// Select a voice.
bool setVoice(char *voice)
{
    unloadVoice();
    return loadVoice(voice);
}

// Set the speech speed.  This can only be done by embedding svox pico specific
// XML tags, so we leave it up to SH.
bool setSpeed(float speed)
{
    return true;
}

// Set the pitch.  0 means default, -100 is min pitch, and 100 is max pitch.
// This can only be done by embedding svox pico specific XML tags, so we leave
// it up to SH.
bool setPitch(float pitch)
{
    return true;
}

// No punctuation level support is directly built into pico.  Let SH deal with
// it.
bool setPunctuationLevel(int level)
{
    return true;
}

// SSML is not directly supported, though a special svox pico tag language is.
bool setSSML(bool value)
{
    return true;
}

// Speak the text.  Block until finished.
bool speakText(char *text)
{
    pico_Int16  textRemaining = strlen(text) + 1; /* includes terminating ‘\0’*/
    pico_Int16 bytesSent, bytesReceived, outDataType;
    pico_Char *inp = (pico_Char *)text;
    short *data;
    unsigned char outBuffer[MAX_OUTBUF_SIZE];
    int status, numSamples;

    while(textRemaining) {
        status = pico_putTextUtf8(picoEngine, inp, textRemaining, &bytesSent);
        textRemaining -= bytesSent;
        inp += bytesSent;
        do {
            status = pico_getData(picoEngine, (void *)outBuffer, MAX_OUTBUF_SIZE - 1, &bytesReceived, &outDataType);
            if(bytesReceived) {
                /* Forward to audio device */
                // TODO: check outDataType and convert outBuffer to short if needed.
                data = (short *)(void *)outBuffer;
                numSamples = bytesReceived >> 1;
                if(!processAudio(data, numSamples)) {
                    pico_resetEngine(picoEngine, PICO_RESET_SOFT);
                    return true; // Abort synthesis
                }
            }
        } while(PICO_STEP_BUSY == status);
    }
    return true;
}

// We don't support variants.
char **getVoiceVariants(int *numVariants)
{
    return NULL;
}

// Select a voice variant.
bool setVoiceVariant(char *variant)
{
    return true;
}
