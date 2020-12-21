/* Sonic library
   Copyright 2010
   Bill Cox
   This file is part of the Sonic Library.

   This file is licensed under the Apache 2.0 license. */

/* Support for reading and writing wave files. */

typedef struct swWaveFileStruct *swWaveFile;

swWaveFile swOpenInputWaveFile(const char *fileName, int *sampleRate, int *numChannels);
swWaveFile swOpenOutputWaveFile(const char *fileName, int sampleRate, int numChannels);
int swCloseWaveFile(swWaveFile file);
int swReadFromWaveFile(swWaveFile file, short *buffer, int maxSamples);
int swWriteToWaveFile(swWaveFile file, const short *buffer, int numSamples);
