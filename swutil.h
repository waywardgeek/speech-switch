// These utilities are provided simply to aid portability.
char **listDirectory(char *dirName, int *numFiles);
char *copyString(char *string);
char *catStrings(char *string1, char *string2);
// This function frees a voice list created with getVoices.
void freeStringList(char **stringList, int numStrings);
char **copyStringList(char **stringList, int numStrings);

// Create a child process and return two FILE objects for communication.  The
// child process simply uses stdin/stdout for communication.  The arguments to
// the child process should be passed as additional parameters, ending with a
// NULL.  Return the child PID.
int swForkWithStdio(char *exePath, FILE **fin, FILE **fout, ...) {
