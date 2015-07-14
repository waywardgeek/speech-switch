#ifndef WIN32
#include <stdbool.h>
#else
#define bool int
#define true 1
#define false 0
#endif

#include "swutil.h"

// Initialize the engine.  synthdataPath is the path to your engine's data directory.
bool initializeEngine(char *synthdataPath);
// Close the TTS Engine.
bool closeEngine(void);
// Return the sample rate in Hz
int getSampleRate(void);
// Return an array of char pointers representing names of supported voices.
// These must allocated with calloc, including the array of pointers, since the
// caller of this function will free them afterwards.  For each voice, there are
// two entries, the voice name, and the language code.  For example, a voice
// name could be "Joey" or "English", and the language code "en-uk".
char **getVoices(int *numVoices);
// Select a voice.
bool setVoice(char *voice);
// Set the speech speed.
bool setSpeed(float speed);
// Set the pitch.
bool setPitch(float pitch);
// Sarts synthesis of the text.  Block until all text is synthesized.
bool speakText(char *text);
// The engine needs to pass the synthesized samples back to the system to be
// processed with this callback.  speakText should block until all samples are
// synthesized.  Samples are in 16-bit signed notation, from -32767 to 32767.
// Returns true to continue synthesis, false to cancel.
bool processAudio(short *data, int numSamples);
// Set the punctuation level for the engine.  By default, it should be set to none.
#define PUNCT_NONE 0
#define PUNCT_SOME 1
#define PUNCT_MOST 2
#define PUNCT_ALL 3
bool setPunctuationLevel(int level);
// Enable or disable support for SSML.  By default, SSML support should be disabled.
bool setSSML(bool value);
// These two functions are only for voices that have "variants", which so far
// means formant synthesizers, specifically espeak, and ibmtts.  Simply return
// NULL if your synthesizer does not have voice variants.
char **getVoiceVariants(int *numVariants);
bool setVoiceVariant(char *variant);

// Some older engines like voxin don't support UTF-8.  These engines should call
// this function from their initializeEngine routine to switch to ANSI.  The
// server will expect the client to use ANSI as well.
void switchToANSI(void);
