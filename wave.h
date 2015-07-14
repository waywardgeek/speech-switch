/* Sonic library
   Copyright 2010
   Bill Cox
   This file is part of the Sonic Library.

   This file is licensed under the Apache 2.0 license. */

/* Support for reading and writing wave files. */

typedef struct WaveFileStruct *WaveFile;

WaveFile openInputWaveFile(char *fileName, int *sampleRate, int *numChannels);
WaveFile openOutputWaveFile(char *fileName, int sampleRate, int numChannels);
int closeWaveFile(WaveFile file);
int readFromWaveFile(WaveFile file, short *buffer, int maxSamples);
int writeToWaveFile(WaveFile file, short *buffer, int numSamples);
