#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <sonic.h>
#include "speechsw.h"
#include "util.h"
#include "wave.h"

#define SONIC_BUFFER_SIZE 2048
#define MAX_PARAGRAPH 2048
#define MIN_PARAGRAPH 1024

static char *swLibDir;

static char *swText;
static uint32_t swTextLen, swTextPos;
static char swParagraph[MAX_PARAGRAPH + SW_MAX_WORD_SIZE];
static bool swConvertToASCII;

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
        "-a             -- Convert text to ASCI before being spoken.\n"
        "-e engine      -- Name of supported engine, like espeak picotts or ibmtts.\n"
        "-f textFile    -- Text file to be spoken.\n"
        "-l             -- List engines.\n"
        "-L             -- List variants available for a given voice.  Use with -v.\n"
        "-p pitch       -- Speech pitch (1.0 is normal).\n"
        "-P             -- Use sonic to adjust pitch rather than the speech engine.\n"
        "-s speed       -- Speech speed (1.0 is normal).\n"
        "-S             -- Use sonic to adjust speed rather than the speech engine.\n"
        "-v voice       -- Name of voice to use.\n"
        "-V variant     -- List variants available for a given voice.\n"
        "-w waveFile    -- Output wave file rather than playing sound.\n");
    exit(1);
}

// Find the location of the lib directory.
static void setDirectories(char *exeName) {
    if (strchr(exeName, '/') == NULL) {
        // Assume it's installed in default location
        const char *libDir = "/usr/lib/speechsw";
        if (access(libDir, R_OK)) {
          libDir = "/usr/lib/speechsw";
        }
        swLibDir = swCopyString(libDir);
    } else {
        // Assume relative to the executable
        const char *relPath = "../lib/speechsw";
        swLibDir = swCalloc(strlen(exeName) + strlen(relPath) + 2, sizeof(char));
        strcpy(swLibDir, exeName);
        char *p = strrchr(swLibDir, '/');
        if (p != NULL) {
          p++;
        } else {
          p = swLibDir;
        }
        strcpy(p, relPath);
    }
    if (access(swLibDir, R_OK)) {
      fprintf(stderr, "Cannot find %s\n", swLibDir);
      exit(1);
    }
}

// Add text to be spoken to the text buffer.
static void addText(char *p) {
    uint32_t len = strlen(p);
    if (swTextPos + len + 1 >= swTextLen) {
        swTextLen <<= 1;
        swText = swRealloc(swText, swTextLen, sizeof(char));
    }
    strcat(swText + swTextPos, p);
    swTextPos += len;
}

// Read text, repacing character sequences < space with a single space.
// Continue until we see a period and space between MIN_PARAGRAPH and
// MAX_PARAGRAPH.  If there is no period in this range, then return
// MAX_PARAGRAPH characters.
static char *readParagraph(FILE *file) {
    uint32_t pos = 0;
    int c = getc(file);
    while(c != EOF && pos < MAX_PARAGRAPH) {
        if (c <= ' ') {
            while(c <= ' ' && c != EOF) {
                c = getc(file);
            }
            if (c != EOF) {
                ungetc(c, file);
            }
            c = ' ';
        }
        if (!swConvertToASCII) {
            swParagraph[pos++] = c;
        } else {
            char *word = swConvertANSIToASCII(c);
            strcpy(swParagraph + pos, word);
            pos += strlen(word);
        }
        if (c == '.' && pos >= MIN_PARAGRAPH) {
            break;
        }
        c = getc(file);
    }
    if (pos == 0 && c == EOF) {
        return NULL;
    }
    swParagraph[pos] = '\0';
    return swParagraph;
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
    if (numSamples == 0) {
        sonicFlushStream(context->sonic);
    } else {
        sonicWriteShortToStream(context->sonic, samples, numSamples);
    }
    uint32_t newNumSamples = sonicSamplesAvailable(context->sonic);
    if (newNumSamples == 0) {
        return 0;
    }
    if (newNumSamples > context->bufferSize) {
        context->bufferSize = newNumSamples << 1;
        context->samples = swRealloc(context->samples, context->bufferSize, sizeof(int16_t));
    }
    if (sonicReadShortFromStream(context->sonic, context->samples, newNumSamples) != newNumSamples) {
        fprintf(stderr, "Error reading from sonic stream\n");
        exit(1);
    }
    return newNumSamples;
}

// This function receives samples from the speech synthesis engine.  If we
// return false, speech synthesis is cancelled.
static bool speechCallback(swEngine engine, int16_t *samples, uint32_t numSamples,
        bool cancelled, void *callbackContext) {
    swContext context = (swContext)callbackContext;
    if (context->sonic != NULL) {
        numSamples = adjustSamples(context, samples, numSamples);
        // adjustSamples writes the new samples to context->samples
        samples = context->samples;
    }
    if (context->outWaveFile != NULL) {
        swWriteToWaveFile(context->outWaveFile, samples, numSamples);
    } else if (context->outStream != NULL) {
        size_t samplesWritten = 0;
        while(samplesWritten < numSamples) {
            samplesWritten += fwrite(samples + samplesWritten, sizeof(int16_t),
                    numSamples - samplesWritten, context->outStream);
        }
    }
    return cancelled;
}

// Speak the text.  Do this in a stream oriented way.
static void speakText(char *waveFileName, char *text, char *textFileName, char *engineName,
        char *voice, char *variant, float speed, float pitch, bool useSonicSpeed,
        bool useSonicPitch) {
    // Start the speech engine
    struct swContextSt context = {0,};
    swEngine engine = swStart(swLibDir, engineName, speechCallback, &context);
    if (engine == NULL) {
        exit(1);
    }
    uint32_t sampleRate = swGetSampleRate(engine);
    if (waveFileName != NULL) {
        // Open the output wave file.
        context.outWaveFile = swOpenOutputWaveFile(waveFileName, sampleRate, 1);
    } else {
        // Play to speaker
        context.outStream = openDefaultSoundDevice(sampleRate);
    }
    if (voice != NULL) {
        swSetVoice(engine, voice);
    }
    if (variant != NULL) {
      if (!swSetVariant(engine, variant)) {
        fprintf(stderr, "Could not set voice variant to %s\n", variant);
      }
    }
    float sonicSpeed = 1.0;
    float sonicPitch = 1.0;
    if (speed != 0.0 && (useSonicSpeed || !swSetSpeed(engine, speed))) {
        useSonicSpeed = true;
        // Map [-100 .. 100] to [1/6 .. 6]
        if (speed > 0.0) {
            sonicSpeed = 1.0 + speed*5.0/100.0;
        } else {
            sonicSpeed = 1.0/(1.0 - speed*5.0/100.0);
        }
    }
    if (pitch != 0.0 && (useSonicPitch || !swSetPitch(engine, pitch))) {
        useSonicPitch = true;
        // Map [-100 .. 100] to [1/2 .. 2]
        if (pitch > 0.0) {
            sonicPitch = 1.0 + pitch/100.0;
        } else {
            sonicPitch = 1.0/(1.0 - pitch/100.0);
        }
    }
    if (useSonicSpeed || useSonicPitch) {
        context.sonic = sonicCreateStream(sampleRate, 1);
        sonicSetSpeed(context.sonic, sonicSpeed);
        sonicSetPitch(context.sonic, sonicPitch);
        context.bufferSize = SONIC_BUFFER_SIZE;
        context.samples = swCalloc(SONIC_BUFFER_SIZE, sizeof(int16_t));
    }
    // TODO: break this into paragraphs
    if (textFileName != NULL) {
        FILE *file = fopen(textFileName, "r");
        if (file == NULL) {
            fprintf(stderr, "Unable to read text file %s\n", textFileName);
            exit(1);
        }
        char *paragraph = readParagraph(file);
        while(paragraph != NULL) {
            // TODO: deal with character encoding
            swSpeak(engine, paragraph, true);
            paragraph = readParagraph(file);
        }
    } else {
        swSpeak(engine, text, true);
    }
    if (useSonicSpeed || useSonicPitch) {
        uint32_t numSamples = adjustSamples(&context, NULL, 0);
        sonicDestroyStream(context.sonic);
        if (numSamples > 0 && !swSpeechCanceled(engine)) {
            // Don't call sonic again
            context.sonic = NULL;
            speechCallback(engine, context.samples, numSamples, false, &context);
        }
        swFree(context.samples);
    }
    if (waveFileName != NULL) {
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
    char *voiceVariant = NULL;
    bool listVariants = false;
    swTextLen = 128;
    swTextPos = 0;
    swText = swCalloc(swTextLen, sizeof(char));
    setDirectories(argv[0]);
    bool useSonicSpeed = false;
    bool useSonicPitch = false;
    swConvertToASCII = false;
    int opt;
    while ((opt = getopt(argc, argv, "ae:f:lLnp:Ps:Sv:V:w:")) != -1) {
        switch (opt) {
        case 'a':
            swConvertToASCII = true;
            break;
        case 'e':
            engineName = optarg;
            break;
        case 'f':
            textFileName = optarg;
            break;
        case 'l': {
            uint32_t numEngines;
            char **engines = swListEngines(swLibDir, &numEngines);
            uint32_t i, j;
            for(i = 0; i < numEngines; i++) {
                swEngine engine = swStart(swLibDir, engines[i], NULL, NULL);
                if (engine != NULL) {
                    printf("%s\n", engines[i]);
                    uint32_t numVoices;
                    char **voices = swListVoices(engine, &numVoices);
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
        case 'L':
            listVariants = true;
            break;
        case 'p':
            pitch = atof(optarg);
            if (pitch > 100.0 || pitch < -100.0) {
                fprintf(stderr, "Pitch must be a floating point value from -100 to 100.\n");
                usage();
            }
            break;
        case 'P':
            useSonicPitch = true;
            break;
        case 's':
            speed = atof(optarg);
            if (speed > 100.0 || speed < -100.0) {
                fprintf(stderr, "Speed must be a floating point value from -100 to 100.\n");
                usage();
            }
            break;
        case 'S':
            useSonicSpeed = true;
            break;
        case 'v':
            voiceName = optarg;
            break;
        case 'V':
            voiceVariant = optarg;
            break;
        case 'w':
            waveFileName = optarg;
            break;
        default: /* '?' */
            fprintf(stderr, "Unknown option %c\n", opt);
            usage();
        }
    }
    if (listVariants) {
        swEngine engine = swStart(swLibDir, engineName, NULL, NULL);
        if (engine == NULL) {
          fprintf(stderr, "Unable to start engine %s\n", engineName);
          return 0;
        }
        uint32_t numVariants;
        char **variants = swGetVariants(engine, &numVariants);
        for(uint32_t i = 0; i < numVariants; i++) {
            printf("%s\n", variants[i]);
        }
        swFreeStringList(variants, numVariants);
        swStop(engine);
        return 0;
    }
    if (optind < argc) {
        if (textFileName != NULL) {
            fprintf(stderr, "Unexpected text on command line while using -f flag\n");
            usage();
        }
        // Speak the command line parameters
        swTextPos = 0;
        uint32_t i;
        for(i = optind; i < argc; i++) {
            addText(argv[i]);
        }
    } else if (textFileName == NULL) {
        addText("Hello, World!");
    }
    speakText(waveFileName, swText, textFileName, engineName, voiceName, voiceVariant, speed,
            pitch, useSonicSpeed, useSonicPitch);
    return 0;
}
