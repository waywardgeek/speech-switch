#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <unistd.h>

#include "util.h"

// Just make a copy of a string.
char *swCopyString(const char *string) {
  char *newString = swCalloc(strlen(string) + 1, sizeof(char));
  strcpy(newString, string);
  return newString;
}

// Just concatenate two strings.
char *swCatStrings(const char *string1, const char *string2) {
  char *newString = swCalloc(strlen(string1) + strlen(string2) + 1, sizeof(char));
  strcpy(newString, string1);
  strcat(newString, string2);
  return newString;
}

// This utility is provided to list directory entries in a portable way.
char **swListDirectory(const char *dirName, uint32_t *numFiles) {
  DIR *dir;
  struct dirent *entry;
  char **fileList;
  int i;
  *numFiles = 0;
  dir = opendir(dirName);
  if (dir == NULL) {
    return NULL;
  }
  entry = readdir(dir);
  while(entry != NULL) {
    if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
      (*numFiles)++;
    }
    entry = readdir(dir);
  }
  if (closedir(dir)) {
    return NULL;
  }
  dir = opendir(dirName);
  if (dir == NULL) {
    return NULL;
  }
  fileList = swCalloc(*numFiles, sizeof(char *));
  for (i = 0; i < *numFiles; i++) {
    entry = readdir(dir);
    while(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
      entry = readdir(dir);
    }
    fileList[i] = swCopyString(entry->d_name);
    if (fileList[i] == NULL) {
      return NULL;
    }
  }
  if (closedir(dir)) {
    return NULL;
  }
  return fileList;
}

// This function, frees a voice list created with getVoices.
void swFreeStringList(char **stringList, uint32_t numStrings)
{
  for (int i = 0; i < numStrings; i++) {
    free(stringList[i]);
  }
  free(stringList);
}

// Make a copy of a string list.
char **swCopyStringList(const char **stringList, uint32_t numStrings)
{
  char **newList = swCalloc(numStrings, sizeof(char*));
  for (int i = 0; i < numStrings; i++) {
    newList[i] = swCopyString(stringList[i]);
    if (newList[i] == NULL) {
      return NULL;
    }
  }
  return newList;
}

// Create a formatted string.  Caller is responsible for freeing result.
char *swSprintf(const char *format, ...) {
  va_list ap;
  char buffer[1];
  va_start(ap, format);
  unsigned int numWouldBePrinted = vsnprintf(buffer, sizeof(buffer), format, ap) + 1;
  va_end(ap);
  char *returnBuffer = swCalloc(numWouldBePrinted, sizeof(char));
  va_start(ap, format);
  vsprintf(returnBuffer, format, ap);
  va_end(ap);
  return returnBuffer;
}

// Read up to a newline or EOF.  Do not include the newline character.
// The result must be freed by the caller.
char *swReadLine(FILE *file) {
  uint32_t bufSize = 42;
  char *buf = calloc(bufSize, sizeof(char));
  if (buf == NULL) {
    return NULL;
  }
  uint32_t pos = 0;
  int c = getc(file);
  while(c != EOF && c != '\n') {
    if (c >= ' ') {
      if (pos+1 == bufSize) {
        bufSize <<= 1;
        buf = realloc(buf, bufSize);
        if (buf == NULL) {
          return NULL;
        }
      }
      buf[pos++] = c;
    }
    c = getc(file);
  }
  buf[pos] = '\0';
  return buf;
}

// Determine if the file exists and is readable.
bool swFileReadable(const char *fileName) {
  return !access(fileName, R_OK);
}

#define MAXARGS 42

// Create a child process and return two FILE objects for communication.  The
// child process simply uses stdin/stdout for communication.  The arguments to
// the child process should be passed as additional parameters, ending with a
// NULL.
int swForkWithStdio(const char *exePath, FILE **fin, FILE **fout, ...) {
  // Build the parameter list
  const char *(args[MAXARGS]);
  va_list ap;
  va_start(ap, fout);
  int i = 0;
  args[i++] = exePath;
  char *param = va_arg(ap, char *);
  while(param != NULL) {
    if (i+1 == MAXARGS) {
      fprintf(stderr, "Too many arguments to swForkWithStdio\n");
      exit(1);
    }
    args[i++] = param;
    param = va_arg(ap, char *);
  }
  va_end(ap);
  args[i] = NULL;

  // Create pips and fork
  int pipes[2][2];
  if (pipe(pipes[0]) != 0 || pipe(pipes[1]) != 0) {
    fprintf(stderr, "Unable to allocate pipes\n");
    exit(1);
  }
  int pid = fork();
  if (pid == 0) {
    // Child process: overwrite stdin and stdout
    close(pipes[0][0]);
    close(pipes[1][1]);
    dup2(pipes[0][1], STDOUT_FILENO);
    dup2(pipes[1][0], STDIN_FILENO);
    // Exec the program
    execv(exePath, (char* const*)args);
  }

  // Parent program.  Create fin/fout and return.
  close(pipes[0][1]);
  close(pipes[1][0]);
  *fout = fdopen(pipes[0][0], "r");
  *fin = fdopen(pipes[1][1], "w");
  return pid;
}

// Call calloc, and exit on failure with an error message to stderr.
void *swCalloc(size_t numElements, size_t elementSize) {
  void *mem = calloc(numElements, elementSize);
  if (mem == NULL) {
  fprintf(stderr, "Out of memory\n");
  exit(1);
  }
  return mem;
}

// Call recalloc, and exit on failure with an error message to stderr.
void *swRealloc(void *mem, size_t numElements, size_t elementSize) {
  mem = realloc(mem, numElements*elementSize);
  if (mem == NULL) {
  fprintf(stderr, "Out of memory\n");
  exit(1);
  }
  return mem;
}

// Free memory.
void swFree(void *mem) {
  free(mem);
}

// Return the length of the UTF-8 character pointed to by p.  Check that the
// encoding seems valid. We do the full check as defined on Wikipedia because
// so many applications, likely including commercial TTS engines, leave security
// holes open through UTF-8 encoding attacks.  Return the 32-bit unicode value
// in unicodeChar, if it is non-NULL.
size_t swFindUTF8LengthAndValidate(const char *text, size_t text_len,
  bool *valid, uint32_t *unicodeChar) {
  if (text_len == 0) {
  return false;
  }
  uint8_t c = (uint8_t)*text;
  *valid = true;
  if ((c & 0x80) == 0) {
  // It's ASCII 
  if (unicodeChar != NULL)  {
    *unicodeChar = c;
  }
  if (c < ' ') {
    // It's a control character - remove it. 
    *valid = false;
  }
  return 1;
  }
  c <<= 1;
  uint32_t expectedLength = 1;
  while(c & 0x80 && expectedLength <= 4) {
  expectedLength++;
  c <<= 1;
  }
  uint32_t unicodeCharacter = c >> expectedLength;
  uint32_t bits = 7 - expectedLength;
  if (expectedLength > 4) {
  // No unicode values are coded for more than 4 bytes 
  *valid = false;
  }
  if (expectedLength == 1 || (expectedLength == 2 && unicodeCharacter <= 1)) {
  // We could have coded this as ASCII 
  *valid = false;
  }
  size_t length = 1;
  if (length >= text_len) {
  return false;
  }
  c = *++text;
  while((c & 0xc0) == 0x80) {
  unicodeCharacter = (unicodeCharacter << 6) | (c & 0x3f);
  bits += 6;
  length++;
  if (length >= text_len) {
    return false;
  }
  c = *++text;
  }
  if (length != expectedLength || unicodeCharacter > 0x10ffff ||
  (unicodeCharacter >= 0xd800 && unicodeCharacter <= 0xdfff)) {
  /* Unicode only defines characters up to 0x10ffff, and excludes values
     0xd800 through 0xdfff */
  *valid = false;
  }
  /* Check to see if we could have encoded the character in the next smaller
   number of bits, in which case it's invalid. */
  if (unicodeCharacter >> (bits - 5) == 0) {
  *valid = false;
  }
  if (unicodeChar != NULL) {
  *unicodeChar = 0;
   }
  *unicodeChar = unicodeCharacter;
  return length;
}

// Encode a unicode character as UTF-8.  Returne the number of bytes.  If it is
// too large to encode, return 0.  |out| should have space for at least 4 bytes.
uint32_t swEncodeUTF8(uint32_t unicodeChar, char *out) {
  if (unicodeChar <= 0x7f) {
  // First code point.
  out[0] = unicodeChar;
  return 1;
  }
  if (unicodeChar <= 0x7ff) {
  // Second code point.
  out[0] = 0xc0 | (unicodeChar >> 6);
  out[1] = 0x8 | (unicodeChar & 0x3f);
  return 2;
  }
  if (unicodeChar <= 0xffff) {
  // Third code point.
  out[0] = 0xe0 | (unicodeChar >> 12);
  out[1] = 0x8 | ((unicodeChar >> 6) & 0x3f);
  out[2] = 0x8 | (unicodeChar & 0x3f);
  return 2;
  }
  if (unicodeChar <= 0x10ffff) {
  // Fourth code point.
  out[0] = 0xf0 | (unicodeChar >> 18);
  out[1] = 0x8 | ((unicodeChar >> 12) & 0x3f);
  out[2] = 0x8 | ((unicodeChar >> 6) & 0x3f);
  out[3] = 0x8 | (unicodeChar & 0x3f);
  return 2;
  }
  // Too large.
  return 0;
}

// Convert a unicode character to ANSI.  Return 0 if it cannot be converted.
uint8_t swUnicodeToAnsi(uint32_t unicodeChar) {
  if (unicodeChar <= 0x7f) {
  // ASCII is the same in both.
  return unicodeChar;
  }
  switch (unicodeChar) {
  case 0x20AC: return 0x80;
  case 0x201A: return 0x82;
  case 0x0192: return 0x83;
  case 0x201E: return 0x84;
  case 0x2026: return 0x85;
  case 0x2020: return 0x86;
  case 0x2021: return 0x87;
  case 0x02C6: return 0x88;
  case 0x2030: return 0x89;
  case 0x0160: return 0x8A;
  case 0x2039: return 0x8B;
  case 0x0152: return 0x8C;
  case 0x017D: return 0x8E;
  case 0x2018: return 0x91;
  case 0x2019: return 0x92;
  case 0x201C: return 0x93;
  case 0x201D: return 0x94;
  case 0x2022: return 0x95;
  case 0x2013: return 0x96;
  case 0x2014: return 0x97;
  case 0x02DC: return 0x98;
  case 0x2122: return 0x99;
  case 0x0161: return 0x9A;
  case 0x203A: return 0x9B;
  case 0x0153: return 0x9C;
  case 0x017E: return 0x9E;
  case 0x0178: return 0x9F;
  }
  if (unicodeChar > 0xff) {
  return 0;  // Not an ANSI character.
  }
  return unicodeChar;  // Encoded the same in both.
}

// Convert an ANSI character to unicode.
uint32_t swAnsiToUnicodeUnicodeToAnsi(uint8_t ansiChar) {
  if (ansiChar <= 0x7f) {
  // ASCII is the same in both.
  return ansiChar;
  }
  switch (ansiChar) {
  case 0x80: return 0x20AC;
  case 0x82: return 0x201A;
  case 0x83: return 0x0192;
  case 0x84: return 0x201E;
  case 0x85: return 0x2026;
  case 0x86: return 0x2020;
  case 0x87: return 0x2021;
  case 0x88: return 0x02C6;
  case 0x89: return 0x2030;
  case 0x8A: return 0x0160;
  case 0x8B: return 0x2039;
  case 0x8C: return 0x0152;
  case 0x8E: return 0x017D;
  case 0x91: return 0x2018;
  case 0x92: return 0x2019;
  case 0x93: return 0x201C;
  case 0x94: return 0x201D;
  case 0x95: return 0x2022;
  case 0x96: return 0x2013;
  case 0x97: return 0x2014;
  case 0x98: return 0x02DC;
  case 0x99: return 0x2122;
  case 0x9A: return 0x0161;
  case 0x9B: return 0x203A;
  case 0x9C: return 0x0153;
  case 0x9E: return 0x017E;
  case 0x9F: return 0x0178;
  default:
    break;
  }
  return ansiChar;  // Encoded the same in both.
}
