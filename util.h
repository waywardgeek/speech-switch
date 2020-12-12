#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// These utilities are provided simply to aid portability.
char **swListDirectory(const char *dirName, uint32_t *numFiles);
// Determine if the file exists and is readable.
bool swFileReadable(const char *fileName);
// Make a copy of the string.  The caller is responsible for calling free.
char *swCopyString(const char *string);
// Concatenate two strings.  The caller is responsible for calling free.
char *swCatStrings(const char *string1, const char *string2);
// Create a formatted string.  Caller is responsible for freeing result.
char *swSprintf(const char *format, ...);
// This function frees a voice list created with getVoices.
void swFreeStringList(char **stringList, uint32_t numStrings);
char **swCopyStringList(const char **stringList, uint32_t numStrings);
// Read up to a newline or EOF.  Do not include the newline character.
// The result must be freed by the caller.
char *swReadLine(FILE *file);

// Call calloc, and exit on failure with an error message to stderr.
void *swCalloc(size_t numElements, size_t elementSize);
// Call recalloc, and exit on failure with an error message to stderr.
void *swRealloc(void *mem, size_t numElements, size_t elementSize);
// Free memory.
void swFree(void *mem);

// Create a child process and return two FILE objects for communication.  The
// child process simply uses stdin/stdout for communication.  The arguments to
// the child process should be passed as additional parameters, ending with a
// NULL.  Return the child PID.
int swForkWithStdio(const char *exePath, FILE **fin, FILE **fout, ...);

// Convert an ANSI character to ASCII.  The returned string is zero-terminated.
// This returns a static buffer and is not thread safe.
char *swConvertANSIToASCII(char c);
// The returned word will be at most this long
#define SW_MAX_WORD_SIZE 16

// Return the length of the UTF-8 character pointed to by p.  Check that the
// encoding seems valid. We do the full check as defined on Wikipedia because
// so many applications, likely including commercial TTS engines, leave security
// holes open through UTF-8 encoding attacks.  Return the 32-bit unicode value
// in unicodeChar, if it is non-NULL.
size_t swFindUTF8LengthAndValidate(const char *text, size_t text_len,
    bool *valid, uint32_t *unicodeChar);
// Encode a unicode character as UTF-8.  Returne the number of bytes.  If it is
// too large to encode, return 0.  |out| should have space for at least 4 bytes.
uint32_t swEncodeUTF8(uint32_t unicodeChar, char *out);
// Convert a unicode character to ANSI.  Return 0 if it cannot be converted.
uint8_t swUnicodeToAnsi(uint32_t unicodeChar);
// Convert an ANSI character to unicode.
uint32_t swAnsiToUnicodeUnicodeToAnsi(uint8_t ansiChar);
