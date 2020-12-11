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
    if(dir == NULL) {
        return NULL;
    }
    entry = readdir(dir);
    while(entry != NULL) {
        if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            (*numFiles)++;
        }
        entry = readdir(dir);
    }
    if (closedir(dir)) {
      return NULL;
    }
    dir = opendir(dirName);
    if(dir == NULL) {
        return NULL;
    }
    fileList = swCalloc(*numFiles, sizeof(char *));
    for(i = 0; i < *numFiles; i++) {
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
    for(int i = 0; i < numStrings; i++) {
        free(stringList[i]);
    }
    free(stringList);
}

// Make a copy of a string list.
char **swCopyStringList(const char **stringList, uint32_t numStrings)
{
    char **newList = swCalloc(numStrings, sizeof(char*));
    for(int i = 0; i < numStrings; i++) {
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
        if(c >= ' ') {
            if(pos+1 == bufSize) {
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
        if(i+1 == MAXARGS) {
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
    if(pipe(pipes[0]) != 0 || pipe(pipes[1]) != 0) {
        fprintf(stderr, "Unable to allocate pipes\n");
        exit(1);
    }
    int pid = fork();
    if(pid == 0) {
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
