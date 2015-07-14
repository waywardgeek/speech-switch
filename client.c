// This interface provides a simple library for talking to the supported voice engines.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "util.h"
#include "speechswitch.h"

#define MAX_TEXT_LENGTH (1 << 16)

struct swEngineSt {
    char *name;
    FILE *fin;
    FILE *fout;
    swCallback callback;
    void *callbackContext;
    int pid;
};

// Write a formatted string to the server.
static void writeServer(
    swEngine engine,
    char *format,
    ...)
{
    va_list ap;
    char buf[MAX_TEXT_LENGTH];

    va_start(ap, format);
    vsnprintf(buf, MAX_TEXT_LENGTH - 1, format, ap);
    va_end(ap);
    buf[MAX_TEXT_LENGTH - 1] = '\0';
    fputs(buf, engine->fin);
    fflush(engine->fin);
}

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
    sprintf(fileName, "%s/sw_%s", enginesDirectory, engineName);
    engine->name = swCopyString(engineName);
    engine->callback = callback;
    engine->callbackContext = callbackContext;
    engine->pid = swForkWithStdio(fileName, &engine->fin, &engine->fout,
        engineDataDirectory, NULL);
    return engine;
}

// Shut down the speech engine, and free the swEngine object.
void swStop(swEngine engine) {
    writeServer(engine, "quit\n");
    fclose(engine->fin);
    fclose(engine->fout);
    free(engine->name);
    free(engine);
}

// TODO: reuse buffers rather than calloc each time.

// Convert a line of hex digits to int16_t.
static void convertHexToInt16(int16_t *samples, char *line, uint32_t numSamples) {
    uint32_t i;
    for(i = 0; i < numSamples; i++) {
        int16_t value = 0;
        uint32_t j;
        for(j = 0; j < 4; j++) {
            value <<= 4;
            char c = *line++;
            uint32_t digit = 0;
            if(c >= '0' && c <= '9') {
                digit = c - '0';
            } else if(c >= 'A' && c <= 'F') {
                digit = c - 'F' + 10;
            } else if(c >= 'a' && c <= 'f') {
                digit = c - 'f' + 10;
            }
            value += digit;
        }
        *samples++ = value;
    }
}

// Read speech data in hexidecimal from the server until "true" is read.
// Caller must free the samples.
static int16_t *readSpeechData(swEngine engine, uint32_t *numSamples) {
    uint32_t bufSize = 1024;
    int16_t *samples = calloc(bufSize, sizeof(int16_t));
    uint32_t pos = 0;
    char *line = swReadLine(engine->fout);
    while(strcmp(line, "true")) {
        uint32_t newSamples = strlen(line)/4;
        if(pos + newSamples > bufSize) {
            bufSize = (pos + newSamples) << 1;
            samples = realloc(samples, bufSize*sizeof(int16_t));
        }
        convertHexToInt16(samples + pos, line, newSamples);
        pos += newSamples;
    }
    *numSamples = pos;
    return samples;
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
    // Now we have to read data and send it to the callback until we read "true".
    uint32_t numSamples;
    int16_t *samples = readSpeechData(engine, &numSamples);
    while(samples != NULL) {
        engine->callback(engine, samples, numSamples, engine->callbackContext);
        free(samples);
        samples = readSpeechData(engine, &numSamples);
    }
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
