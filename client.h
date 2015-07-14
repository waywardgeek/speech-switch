// This interface provides a light weight C interface for talking to the
// supported voice engines.

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SW_UTF8,
    SW_ANSI
} swEncoding;

typedef enum {
    PUNCT_NONE=0,
    PUNCT_SOME=1,
    PUNCT_MOST=2,
    PUNCT_ALL=3
} swPuncuationLevel;

struct swEngineSt;

typedef struct swEngineSt *swEngine;

typedef bool (*callback)(swEngine engine, uint16_t *samples, uint32_t numSamples)) swCallback;

// These functions start/stop engines and synthesize speech.

// List available engines.
char **swListEngines(char *enginesDirectory);
// Create and initialize a new swEngine object, and connect to the speech engine.
swEngine swStart(char *enginesDirectory, char *engineName, char *engineDataDirectory,
        swCallback callback);
// Shut down the speech engine, and free the swEngine object.
void swStop(swEngine engine);
// Synthesize speech samples.  Synthesized samples will be passed to the 
// callback function passed to swStart.  To continue receiving samples, the
// callback should return true.  Returning false will cancel further speech.
bool swSpeak(swEngine engine, char *text, bool isUTF8);

// These fucntions control speech synthesis parameters.

// Interrupt speech while being synthesized.
void swCancel(swEngine engine);
// Get the sample rate in Hertz.
uint32_t swGetSampleRate(swEngine engine);
// Get a list of supported voices.  The caller can call swFreeStrings to free
// them.
char **swGetVoices     - List available voices
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
swSetSpeed(swEngine engine, float speed);
// Enable or disable ssml support.
bool swSetSSML(swEngine engine, bool enable);
// Return the protocol version, Currently 1 for all engines.
uint32_t swGetVersion(swEngine engine);
