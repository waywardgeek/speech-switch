#include "client.h"

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#incldue <sonic.h>

static char *text;
static uint32_t textLen, textPos;

// Report usage flags and exit.
static void usage(void) {
    fprintf(stderr,
        "Supported flags are:\n"
        "\n"
        "-e engine      -- name of supported engine, like espeak picotts or ibmtts\n"
        "-f textFile    -- text file to be spoken\n"
        "-p pitch       -- speech pitch (1.0 is normal)\n"
        "-s speed       -- speech speed (1.0 is normal)\n"
        "-v voice       -- name of voice to use\n"
        "-w waveFile    -- output wave file rather than playing sound\n");
    exit(1);
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

// Speak the text.  Do this in a stream oriented way.
static speakText(char *waveFileName, char *text, char *textFileName,
        char *engine, char *voice, double speed, double pitch) {
    if(waveFileName == NULL) {
        fprintf(stderr, "Currently, you must supply the -w flag\n");
        return 1;
    }
    // Open the output wave file.
    waveFile outfile = openOutputWaveFile(waveFileName, sampleRate, 1);
    uint32_t numSamples, sampleRate;
    int16_t samples = genSamples(text, engine, &numSamples, &sampleRate);
    // Now change the speed and pitch
    if(speed < 1.0) {
        samples = realloc(samples, (size_t)(2*2*1.1*numSamples/speed));
    }
    uint32_t numSamples = sonicChangeShortSpeed(samples, numSamples, speed, pitch,
        1.0, 1.0, false, sampleRate, 1);
    // Now play it
    system("sw_play -r %d -f
    free(samples);

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
    addText("Hello, World!");
    int opt;
    while ((opt = getopt(argc, argv, "e:f:p:s:v:w:")) != -1) {
        switch (opt) {
        case 'e':
            engineName = optarg;
            break;
        case 'f':
            textFileName = optarg;
            break;
        case 'p':
            pitch = atof(optarg);
            if(pitch == 0.0) {
                fprintf(stderr, "Pitch must be a floating point value.\n"
                    "2.0 is twice the pitch , 0.5 is half\");
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
        if(textFile != NULL) {
            fprintf("Unexpected text on command line while using -f flag\n");
            usage();
        }
        // Speak the command line parameters
        textPos = 0;
        uint32_t i;
        for(i = optind; i < argc; i++) {
            addText(argv[i]);
        }
    }
    speakText(waveFileName, text, textFileName, engine, voice, speed, pitch);
    return 0;
}
