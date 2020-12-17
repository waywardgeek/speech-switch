#ifndef WIN32
#include <stdbool.h>
#else
#define bool int
#define true 1
#define false 0
#endif
#include <stdint.h>

#include "util.h"

// Initialize the engine.  synthdataPath is the path to your engine's data directory.
bool swInitializeEngine(const char *synthdataPath);
// Close the TTS Engine.
bool swCloseEngine(void);
// Return the sample rate in Hz
uint32_t swGetSampleRate(void);
// Return an array of char pointers representing names of supported voices.
// These must allocated with calloc, including the array of pointers, since the
// caller of this function will free them afterwards.  For each voice, there are
// two entries, the voice name, and the language code.  For example, a voice
// name could be "Joey" or "English", and the language code "en-uk".
char **swGetVoices(uint32_t *numVoices);
// If the TTS engine generally sounds better when sped up by Sonic, return true.
bool swUseSonicSpeed(void);
// If the TTS engine generally sounds better when pitch is adjusted by Sonic,
// return true.
bool swUseSonicPitch(void);
// Select a voice.
bool swSetVoice(const char *voice);
// Set the speech speed.  Not required if using Sonic for speed.
bool swSetSpeed(float speed);
// Set the pitch.  Not required if using Sonic for pitch.
bool swSetPitch(float pitch);
// Sarts synthesis of the text.  Block until all text is synthesized.
bool swSpeakText(const char *text);
// Speak a character name.
bool swSpeakChar(uint32_t unicodeChar);
// The engine needs to pass the synthesized samples back to the system to be
// processed with this callback.  speakText should block until all samples are
// synthesized.  Samples are in 16-bit signed notation, from -32767 to 32767.
// Returns true to continue synthesis, false to cancel.
bool swProcessAudio(const short *data, int numSamples);
// Set the punctuation level for the engine.  By default, it should be set to none.
typedef enum {
    PUNCT_NONE = 0,
    PUNCT_SOME = 1,
    PUNCT_MOST = 2,
    PUNCT_ALL = 3
} swPunctLevel;
bool swSetPunctuationLevel(swPunctLevel level);
// Enable or disable support for SSML.  By default, SSML support should be disabled.
bool swSetSSML(bool value);
// These two functions are only for voices that have "variants", which so far
// means formant synthesizers, specifically espeak, and ibmtts.  Simply return
// NULL if your synthesizer does not have voice variants.
char **swGetVoiceVariants(uint32_t *numVariants);
bool swSetVoiceVariant(const char *variant);

// Some older engines like voxin don't support UTF-8.  These engines should call
// this function from their initializeEngine routine to switch to ANSI.  The
// server will expect the client to use ANSI as well.
void swSwitchToANSI(void);
// Convert a factor that changes pitch or speed, to minRange .. maxRange.
int swFactorToRange(float factor, float minFactor, float maxFactor, int minRange,
    int defaultRange, int maxRange);
