// This interface provides a simple library for talking to the supported voice engines.

struct swEngineSt;
typedef swEngineSt *swEngine;

swEngine swStartEngine(char *engineName);
void swStopEngine(swEngine engine);
uint32_t swEngineGetSampleRate(swEngine engine);
uint32_t swEngineSetSpeechCallback(swEngine,
    void (*callback)(swEngine engine, uint16_t *samples, uint32_t numSamples));
void swEngineSpeak(char *text, bool isUTF8);
