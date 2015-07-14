#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <sonic.h>
#include "util.h"
#include "speechswitch.h"

static char *swEngineDir;

static char *text;
static uint32_t textLen, textPos;

// Report usage flags and exit.
static void usage(void) {
    fprintf(stderr,
        "Supported flags are:\n"
        "\n"
        "-e engine      -- name of supported engine, like espeak picotts or ibmtts\n"
        "-f textFile    -- text file to be spoken\n"
        "-l             -- list engines\n"
        "-p pitch       -- speech pitch (1.0 is normal)\n"
        "-s speed       -- speech speed (1.0 is normal)\n"
        "-v voice       -- name of voice to use\n"
        "-w waveFile    -- output wave file rather than playing sound\n");
    exit(1);
}

struct swContextSt {
    swWaveFile outWaveFile;
    FILE *outStream;
};

typedef struct swContextSt *swContext;

// Add text to be spoken to the text buffer.
static void addText(char *p) {
    uint32_t len = strlen(p);
    if(textPos + len + 1 >= textLen) {
        textLen <<= 1;
        text = realloc(text, textLen*sizeof(char));
    }
    strcat(text + textPos, p);
    textPos += len;
}

// Play the sound samples.  We basically put the problem to aplay.  We use:
//   paplay --raw --rate=<samplerate> --channels=1 --format=s16le
static FILE *openDefaultSoundDevice(uint32_t sampleRate) {
    FILE *fin, *fout;
    char rateParam[42];
    sprintf(rateParam, "--rate=%u", sampleRate);
    swForkWithStdio("/usr/bin/paplay", &fin, &fout, "--raw", rateParam, "--channels=1",
            "--format=s16le", NULL);
    return fin;
}

// This function receives samples from the speech synthesis engine.  If we
// return false, speech synthesis is cancelled.
static bool speechCallback(swEngine engine, int16_t *samples, uint32_t numSamples,
        void *callbackContext) {
    swContext context = (swContext)callbackContext;
    if(context->outWaveFile != NULL) {
        swWriteToWaveFile(context->outWaveFile, samples, numSamples);
    } else if(context->outStream != NULL) {
        uint32_t totalWritten = 0;
        while(totalWritten < numSamples) {
            uint32_t numWritten = fwrite(samples, sizeof(int16_t), numSamples, context->outStream);
            samples += numWritten;
            totalWritten += numWritten;
        }
    }
    return true;
}

// Speak the text.  Do this in a stream oriented way.
static void speakText(char *waveFileName, char *text, char *textFileName,
        char *engineName, char *voice, double speed, double pitch) {
    // Start the speech engine
    // TODO: deal with data directory
    struct swContextSt context = {0,};
    swEngine engine = swStart(swEngineDir, engineName, NULL, speechCallback, &context);
    uint32_t sampleRate = swGetSampleRate(engine);
    if(waveFileName != NULL) {
        // Open the output wave file.
        context.outWaveFile = swOpenOutputWaveFile(waveFileName, sampleRate, 1);
    } else {
        // Play to speaker
        context.outStream = openDefaultSoundDevice(sampleRate);
    }
    swSetSpeed(engine, speed);
    swSetPitch(engine, pitch);
    swSpeak(engine, text, true);
    if(waveFileName != NULL) {
        swCloseWaveFile(context.outWaveFile);
    } else {
        fclose(context.outStream);
    }
}

int main(int argc, char *argv[]) {
    char *engineName = "espeak"; // It's always there!
    char *textFileName = NULL;
    double speed = 1.0;
    double pitch = 1.0;
    char *waveFileName = NULL;
    char *voiceName = NULL;
    textLen = 128;
    textPos = 0;
    text = calloc(textLen, sizeof(char));
    char *relPath = "/../lib/speechswitch/engines";
    swEngineDir = calloc(strlen(argv[0]) + strlen(relPath) + 1, sizeof(char));
    strcpy(swEngineDir, argv[0]);
    char *p = strrchr(swEngineDir, '/');
    *p = '\0';
    strcat(swEngineDir, relPath);
    int opt;
    while ((opt = getopt(argc, argv, "e:f:lp:s:v:w:")) != -1) {
        switch (opt) {
        case 'e':
            engineName = optarg;
            break;
        case 'f':
            textFileName = optarg;
            break;
        case 'l':
            {
            uint32_t numEngines;
            char ** engines = swListEngines(swEngineDir, &numEngines);
            uint32_t i, j;
            for(i = 0; i < numEngines; i++) {
                swEngine engine = swStart(swEngineDir, engines[i], NULL, NULL, NULL);
                if(engine != NULL) {
                    printf("%s\n", engines[i]);
                    uint32_t numVoices;
                    char **voices = swGetVoices(engine, &numVoices);
                    for(j = 0; j < numVoices; j++) {
                        printf("    %s\n", voices[j]);
                    }
                    swFreeStringList(voices, numVoices);
                    swStop(engine);
                }
            }
            swFreeStringList(engines, numEngines);
            return 0;
            }
        case 'p':
            pitch = atof(optarg);
            if(pitch == 0.0) {
                fprintf(stderr, "Pitch must be a floating point value.\n"
                    "2.0 is twice the pitch , 0.5 is half\n");
                usage();
            }
            break;
        case 's':
            speed = atof(optarg);
            if(speed == 0.0) {
                fprintf(stderr, "Speed must be a floating point value.\n"
                    "2.0 speeds up by 2X, 0.5 slows down by 2X.\n");
                usage();
            }
            break;
        case 'v':
            voiceName = optarg;
            break;
        case 'w':
            waveFileName = optarg;
            break;
        default: /* '?' */
            fprintf(stderr, "Unknown option %c\n", opt);
            usage();
        }
    }
    if (optind < argc) {
        if(textFileName != NULL) {
            fprintf(stderr, "Unexpected text on command line while using -f flag\n");
            usage();
        }
        // Speak the command line parameters
        textPos = 0;
        uint32_t i;
        for(i = optind; i < argc; i++) {
            addText(argv[i]);
        }
    } else if(textFileName != NULL) {
        addText("Hello, World!");
    }
    speakText(waveFileName, text, textFileName, engineName, voiceName, speed, pitch);
    return 0;
}
