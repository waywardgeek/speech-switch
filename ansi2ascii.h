// Convert an ANSI character to ASCII.  The returned string is zero-terminated.
// This returns a static buffer and is not thread safe.
char *swConvertANSIToASCII(char c);
// Convert an entire ANSI string to ASCII.  Caller must free the returned value.
char *swConvertANSIStringToASCII(char *text);
// The returned word will be at most this long
