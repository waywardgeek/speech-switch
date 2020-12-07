// This interface provides a simple library for talking to the supported voice engines.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "util.h"
#include "speechsw.h"

#define MAX_TEXT_LENGTH (1 << 16)

struct swEngineSt {
    char *name;
    FILE *fin;
    FILE *fout;
    swCallback callback;
    void *callbackContext;
    int pid;
    volatile bool cancel;
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

// Write "true\n" or "false\n" to the server.
static void writeBool(swEngine engine, bool value) {
    if(value) {
        writeServer(engine, "true\n");
    } else {
        writeServer(engine, "false\n");

    }
}

// List available engines.
char **swListEngines(char *enginesDirectory, uint32_t *numEngines) {
    char **engines = swListDirectory(enginesDirectory, numEngines);
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
    char *fileName = calloc(strlen(enginesDirectory) + strlen(engineName) + 5, 1);
    sprintf(fileName, "%s/sw_%s", enginesDirectory, engineName);
    if(!swFileReadable(fileName)) {
        fprintf(stderr, "Unable to execute %s\n", fileName);
        free(fileName);
        return NULL;
    }
    swEngine engine = calloc(1, sizeof(struct swEngineSt));
    engine->name = swCopyString(engineName);
    engine->callback = callback;
    engine->callbackContext = callbackContext;
    engine->pid = swForkWithStdio(fileName, &engine->fin, &engine->fout,
        engineDataDirectory, NULL);
    free(fileName);
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
        fprintf(stderr, "Hex digits left over: %u", numDigits);
    }
    return numSamples;
}

// Read speech data in hexidecimal from the server until "true" is read.
// Caller must free the samples.
static int16_t *readSpeechData(swEngine engine, uint32_t *numSamples) {
    char *line = swReadLine(engine->fout);
    if(!strcmp(line, "true")) {
        // We're done.
        *numSamples = 0;
        free(line);
        return NULL;
    }
    uint32_t bufSize  = strlen(line)/4;
    int16_t *samples = calloc(bufSize, sizeof(int16_t));
    *numSamples = convertHexToInt16(samples, line);
    free(line);
    return samples;
}

// Synthesize speech samples.  Synthesized samples will be passed to the 
// callback function passed to swStart.  To continue receiving samples, the
// callback should return true.  Returning false will cancel further speech.
bool swSpeak(swEngine engine, char *text, bool isUTF8) {
    // TODO: deal with isUTF8
    // TODO: replace any \n. with \n..
    engine->cancel = false;
    fputs("speak\n", engine->fin);
    fputs(text, engine->fin);
    writeServer(engine, "\n.\n");
    // TODO: deal with error handling
    // Now we have to read data and send it to the callback until we read "true".
    uint32_t numSamples;
    int16_t *samples = readSpeechData(engine, &numSamples);
    bool cancelled = false;
    while(samples != NULL && !cancelled) {
        cancelled = engine->callback(engine, samples, numSamples, engine->cancel,
                engine->callbackContext);
        free(samples);
        writeBool(engine, !cancelled);
        samples = readSpeechData(engine, &numSamples);
    }
    return !cancelled;
}

// Read a uint32_t from the server.
static uint32_t readUint32(swEngine engine) {
    char *line = swReadLine(engine->fout);
    uint32_t result = atoi(line);
    free(line);
    return result;
}

// Read a count-prefixed string list from the server.
static char **readStringList(swEngine engine, uint32_t *numStrings) {
    *numStrings = readUint32(engine);
    char **strings = calloc(*numStrings, sizeof(char *));
    uint32_t i;
    for(i = 0; i < *numStrings; i++) {
        strings[i] = swReadLine(engine->fout);
    }
    return strings;
}

// Get the sample rate in Hertz.
uint32_t swGetSampleRate(swEngine engine) {
    writeServer(engine, "get samplerate\n");
    return readUint32(engine);
}

// Get a list of supported voices.  The caller can call swFreeStrings
char **swGetVoices(swEngine engine, uint32_t *numVoices) {
    writeServer(engine, "get voices\n");
    return readStringList(engine, numVoices);
}

// Read a line and return true only if the line is "true".
static bool expectTrue(swEngine engine) {
    char *line = swReadLine(engine->fout);
    bool result = !strcmp(line, "true");
    free(line);
    return result;
}

// Set the speech speed.  Speed is from -100.0 to 100.0, and 0 is the default.
bool swSetSpeed(swEngine engine, float speed) {
    writeServer(engine, "set speed %f\n", speed);
    return expectTrue(engine);
}

// Set the pitch.  0 means default, -100 is min pitch, and 100 is max pitch.
bool swSetPitch(swEngine engine, float pitch) {
    writeServer(engine, "set pitch %f\n", pitch);
    return expectTrue(engine);
}

// Select a voice by it's identifier
bool swSetVoice(swEngine engine, char *voice) {
    writeServer(engine, "set voice %s\n", voice);
    return expectTrue(engine);
}

// Return the engine's native encoding.
swEncoding swGetEncoding(swEngine engine) {
    writeServer(engine, "get encoding\n");
    char *line = swReadLine(engine->fout);
    swEncoding encoding = SW_UTF8;
    if(!strcmp(line, "ANSI")) {
        encoding = SW_ANSI;
    }
    free(line);
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
    writeServer(engine, "get variants\n");
    return readStringList(engine, numVariants);
}

// Select a voice variant by it's identifier
bool swSetVariant(swEngine engine, char *variant) {
    writeServer(engine, "set variant %s\n", variant);
    return expectTrue(engine);
}

// Set the punctuation level: none, some, most, or all.
bool swSetPunctuation(swEngine engine, swPunctuationLevel level) {
    char *levelName = NULL;
    switch(level) {
    case PUNCT_NONE: levelName = "none"; break;
    case PUNCT_SOME: levelName = "some"; break;
    case PUNCT_MOST: levelName = "most"; break;
    case PUNCT_ALL: levelName = "all"; break;
    default:
        fprintf(stderr, "Unknonwn punctuation level");
        exit(1);
    }
    writeServer(engine, "set punctuation %s\n", levelName);
    return expectTrue(engine);
}

// Enable or disable ssml support.
bool swSetSSML(swEngine engine, bool enable) {
    writeServer(engine, "set ssml %s\n", enable? "true" : "false");
    return expectTrue(engine);
}

// Return the protocol version, Currently 1 for all engines.
uint32_t swGetVersion(swEngine engine) {
    writeServer(engine, "get version\n");
    return readUint32(engine);
}
