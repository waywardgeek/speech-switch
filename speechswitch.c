#include <stdio.h>
#include <pthread.h>
#include <getopt.h>

static void usage(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, (char *)format, ap);
    va_end(ap);
    fprintf(stderr, "\nUsage: speechswitch [OPTIONS]\n");
    exit(1);
}

// The write message loop.  All messages are written from this thread function
// so that we don't mix them together.
static void *writeMessages(void *contextPtr) {
    int rc = pthread_create(&writeThread, NULL, writeMessages, NULL);

// Start the write thread, that writes messages to clients.
static void startWriteThread(void) {
    pthread_t writeThread;
    int rc = pthread_create(&writeThread, NULL, writeMessages, NULL);
}

// Parse commands and execute them.  
static void runServer(void) {
}

int main(int argc, char **argv) {
    char c;
    while((c = getopt(argc, argv, "")) != -1) {
        switch (c) {
        default:
            usage("Invalid argument");
        }
    }
    startWriteThread();
    runServer();
    return 0;
}
