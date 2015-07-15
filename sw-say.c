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

#define SONIC_BUFFER_SIZE 2048

static char *swEngineDir;

static char *text;
static uint32_t textLen, textPos;

struct swContextSt {
    swWaveFile outWaveFile;
    FILE *outStream;
    sonicStream sonic;
    int16_t *samples; // Used only with libsonic for resizing sample buffer
    uint32_t bufferSize;
};

typedef struct swContextSt *swContext;

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

// Find the location of the engines directory.
static void setEnginesDir(char *exeName) {
    if(strchr(exeName, '/') == NULL) {
        // Assume it's installed in default location
        swEngineDir = "/usr/lib/speechswitch/engines";
    } else {
        // Assume relative to the executable
        char *relPath = "/../lib/speechswitch/engines";
        swEngineDir = calloc(strlen(exeName) + strlen(relPath) + 1, sizeof(char));
        strcpy(swEngineDir, exeName);
        char *p = strrchr(swEngineDir, '/');
        *p = '\0';
        strcat(swEngineDir, relPath);
    }
}

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

// Run sonic to adjust speed and/or pitch
static uint32_t adjustSamples(swContext context, int16_t *samples, uint32_t numSamples) {
    if(numSamples == 0) {
        sonicFlushStream(context->sonic);
    } else {
        sonicWriteShortToStream(context->sonic, samples, numSamples);
    }
    uint32_t newNumSamples = sonicSamplesAvailable(context->sonic);
    if(newNumSamples == 0) {
        return 0;
    }
    if(newNumSamples > context->bufferSize) {
        context->bufferSize = newNumSamples << 1;
        context->samples = realloc(context->samples, context->bufferSize*sizeof(int16_t));
    }
    if(newNumSamples > 0) {
        sonicReadShortFromStream(context->sonic, context->samples, newNumSamples);
    }
    return newNumSamples;
}

// This function receives samples from the speech synthesis engine.  If we
// return false, speech synthesis is cancelled.
static bool speechCallback(swEngine engine, int16_t *samples, uint32_t numSamples,
        void *callbackContext) {
    swContext context = (swContext)callbackContext;
    if(context->sonic != NULL) {
        numSamples = adjustSamples(context, samples, numSamples);
        samples = context->samples;
    }
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
        char *engineName, char *voice, float speed, float pitch) {
    // Start the speech engine
    // TODO: deal with data directory
    struct swContextSt context = {0,};
    swEngine engine = swStart(swEngineDir, engineName, NULL, speechCallback, &context);
    if(engine == NULL) {
        exit(1);
    }
    uint32_t sampleRate = swGetSampleRate(engine);
    if(waveFileName != NULL) {
        // Open the output wave file.
        context.outWaveFile = swOpenOutputWaveFile(waveFileName, sampleRate, 1);
    } else {
        // Play to speaker
        context.outStream = openDefaultSoundDevice(sampleRate);
    }
    if(voice != NULL) {
        swSetVoice(engine, voice);
    }
    bool useSonic = false;
    float sonicSpeed = 1.0;
    float sonicPitch = 1.0;
    if(speed != 0.0 && !swSetSpeed(engine, speed)) {
        useSonic = true;
        // Map [-100 .. 100] to [1.0/-6.0 .. 6.0]
        if(speed > 0.0) {
            sonicSpeed = speed*6.0/100.0;
        } else {
            sonicSpeed = 1.0/(1.0 - speed*5.0/100.0);
        }
    }
    if(pitch != 0.0 && !swSetPitch(engine, pitch)) {
        useSonic = true;
        // Map [-100 .. 100] to [1.0/-6.0 .. 6.0]
        if(speed > 0.0) {
            sonicSpeed = speed*6.0/100.0;
        } else {
            sonicSpeed = 1.0/(1.0 - speed*5.0/100.0);
        }
    }
    if(useSonic) {
        context.sonic = sonicCreateStream(sampleRate, 1);
        sonicSetSpeed(context.sonic, sonicSpeed);
        sonicSetPitch(context.sonic, sonicPitch);
        context.bufferSize = SONIC_BUFFER_SIZE;
        context.samples = calloc(SONIC_BUFFER_SIZE, sizeof(int16_t));
    }
    // TODO: break this into paragraphs
    swSpeak(engine, text, true);
    if(useSonic) {
        uint32_t numSamples = adjustSamples(&context, NULL, 0);
        sonicDestroyStream(context.sonic);
        if(numSamples > 0) {
            // Don't call sonic again
            context.sonic = NULL;
            speechCallback(engine, context.samples, numSamples, &context);
        }
        free(context.samples);
    }
    if(waveFileName != NULL) {
        swCloseWaveFile(context.outWaveFile);
    } else {
        fclose(context.outStream);
    }
}

int main(int argc, char *argv[]) {
    char *engineName = "espeak"; // It's always there!
    char *textFileName = NULL;
    float speed = 0.0;
    float pitch = 0.0;
    char *waveFileName = NULL;
    char *voiceName = NULL;
    textLen = 128;
    textPos = 0;
    text = calloc(textLen, sizeof(char));
    setEnginesDir(argv[0]);
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
            if(pitch > 100.0 || pitch < -100.0) {
                fprintf(stderr, "Pitch must be a floating point value from -100 to 100.\n");
                usage();
            }
            break;
        case 's':
            speed = atof(optarg);
            if(speed > 100.0 || speed < -100.0) {
                fprintf(stderr, "Speed must be a floating point value from -100 to 100.\n");
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
    } else if(textFileName == NULL) {
        addText("Hello, World!");
    }
    speakText(waveFileName, text, textFileName, engineName, voiceName, speed, pitch);
    return 0;
}
