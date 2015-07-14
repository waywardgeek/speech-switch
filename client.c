// This interface provides a simple library for talking to the supported voice engines.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "speechswitch.h"

struct swEngineSt {
    char *name;
    FILE *fin;
    FILE *fout;
    swCallback callback;
    void *callbackContext;
    int pid;
};

// List available engines.
char **swListEngines(char *enginesDirectory, uint32_t *numEngines) {
    uint32_t numFiles;
    char **engines = swListDirectory(enginesDirectory, &numFiles);
    uint32_t i;
    // Trim off thw "sw_" prefix if present
    for(i = 0; i < *numEngines; i++) {
        if(strncmp(engines[i], "sw_", 3) == 0) {
            char *p = engines[i];
            char *q = p + 3;
            while((*p++ = *q++));
        }
    }
    return engines;
}

// Create and initialize a new swEngine object, and connect to the speech engine.
swEngine swStart(char *enginesDirectory, char *engineName, char *engineDataDirectory,
        swCallback callback, void *callbackContext) {
    swEngine engine = calloc(1, sizeof(struct swEngineSt));
    char *fileName = calloc(strlen(enginesDirectory) + strlen(engineName) + 5, 1);
    sprintf("%s/sw_%s", enginesDirectory, engineName);
    engine->name = swCopyString(engineName);
    engine->callback = callback;
    engine->callbackContext = callbackContext;
    engine->pid = swForkWithStdio(fileName, &engine->fin, &engine->fout,
        engineDataDirectory, NULL);
    return engine;
}

// Shut down the speech engine, and free the swEngine object.
void swStop(swEngine engine) {
    fputs("quit\n", engine->fin);
    fclose(engine->fin);
    fclose(engine->fout);
    free(engine->name);
    free(engine);
}

// Synthesize speech samples.  Synthesized samples will be passed to the 
// callback function passed to swStart.  To continue receiving samples, the
// callback should return true.  Returning false will cancel further speech.
bool swSpeak(swEngine engine, char *text, bool isUTF8) {
    // TODO: deal with isUTF8
    // TODO: replace any \n. with \n..
    fputs("speak\n", engine->fin);
    fputs(text, engine->fin);
    fputs(".\n", engine->fin);
    // TODO: deal with error handling
    return true;
}

// Get the sample rate in Hertz.
uint32_t swGetSampleRate(swEngine engine) {
    fputs("get samplerate\n", engine->fin);
    char *line = swReadLine(engine->fout);
    uint32_t result = atoi(line);
    free(line);
    return result;
}

// Interrupt speech while being synthesized.
void swCancel(swEngine engine);
// Get a list of supported voices.  The caller can call swFreeStrings
char **swGetVoices(swEngine engine);
// Free string lists returned by swGetVoices or swGetVariants
void swFreeStringList(char **stringList, uint32_t numStrings);
// List available variations on voices.
char **swGetVariants(swEngine engine);
// Return the encoding.
swEncoding swGetEncoding(swEngine engine);
// Select a voice by it's identifier
bool swSetVoice(swEngine engine, char *voice);
// Select a voice variant by it's identifier
bool swSetVariant(swEngine engine, char *variant);
// Set the pitch.  0 means default, -100 is min pitch, and 100 is max pitch.
bool swSetPitch(swEngine engine, float pitch);
// Set the punctuation level: none, some, most, or all.
bool swSetPunctuation(swPunctuationLevel level);
// Set the speech speed.  Speed is from -100.0 to 100.0, and 0 is the default.
bool swSetSpeed(swEngine engine, float speed);
// Enable or disable ssml support.
bool swSetSSML(swEngine engine, bool enable);
// Return the protocol version, Currently 1 for all engines.
uint32_t swGetVersion(swEngine engine);
