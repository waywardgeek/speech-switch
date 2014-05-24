#include <stdbool.h>
#include <stdio.h>
#include <iconv.h>

/* Return the length of the UTF-8 character pointed to by p.  Check that the
   encoding seems valid. We do the full check as defined on Wikipedia because
   so many applications, likely including commercial TTS engines, leave security
   holes open through UTF-8 encoding attacks. */
static int findLengthAndValidate(char *p, bool *valid)
{
    int length, expectedLength, bits;
    unsigned long unicodeCharacter;
    unsigned char c = (unsigned char)*p;

    *valid = true;
    if((c & 0x80) == 0) {
        // It's ASCII 
        /* temp: Commented out for matching iconv
        if(c < ' ') {
            // It's a control character - remove it. 
            *valid = false;
        }
        */
        return 1;
    }
    c <<= 1;
    expectedLength = 1;
    while(c & 0x80) {
        expectedLength++;
        c <<= 1;
    }
    unicodeCharacter = c >> expectedLength;
    bits = 7 - expectedLength;
    if(expectedLength > 4 || expectedLength == 1) {
        // No unicode values are coded for more than 4 bytes 
        *valid = false;
    }
    if(expectedLength == 1 || (expectedLength == 2 && unicodeCharacter <= 1)) {
        // We could have coded this as ASCII 
        *valid = false;
    }
    length = 1;
    c = *++p;
    while((c & 0xc0) == 0x80) {
        unicodeCharacter = (unicodeCharacter << 6) | (c & 0x3f);
        bits += 6;
        length++;
        c = *++p;
    }
    if(length != expectedLength || unicodeCharacter > 0x10ffff ||
        (unicodeCharacter >= 0xd800 && unicodeCharacter <= 0xdfff)) {
        /* Unicode only defines characters up to 0x10ffff, and excludes values
           0xd800 through 0xdfff */
        *valid = false;
    }
    /* Check to see if we could have encoded the character in the next smaller
       number of bits, in which case it's invalid. */
    if(unicodeCharacter >> (bits - 5) == 0) {
        *valid = false;
    }
    return length;
}

int main()
{
    iconv_t cd = iconv_open("UTF-32", "UTF-8");
    unsigned int value = 0;
    size_t inlength, outlength;
    unsigned char inbuf[10], outbuf[50];
    char *inp, *outp;
    size_t result;
    bool iconvValid, meValid;
    int length;
    bool randFlipMe, randFlipIconv;

    srand(123456789);
    inbuf[4] = 0;
    inbuf[5] = 0;
    inbuf[6] = 0;
    inbuf[7] = 0;
    inbuf[8] = 0;
    inbuf[9] = 0;
    do {
        // Let's force them to disagree now and then and make sure we catch it.
        randFlipMe = (rand() & 0xfffff) == 0;
        randFlipIconv = (rand() & 0xfffff) == 0;
        inlength = 10;
        outlength = 50;
        inp = inbuf;
        outp = outbuf;
        inbuf[0] = value & 0xff;
        inbuf[1] = (value >> 8) & 0xff;
        inbuf[2] = (value >> 16) & 0xff;
        inbuf[3] = (value >> 24) & 0xff;
        result = iconv(cd, &inp, &inlength, &outp, &outlength);
        iconvValid = true;
        if(result == -1) {
            iconvValid = false;
        }
        iconvValid ^= randFlipIconv;
        inp = inbuf;
        do {
            length = findLengthAndValidate(inp, &meValid);
            inp += length;
        } while(meValid && inp - (char *)inbuf < 4);
        meValid ^= randFlipMe;
        if(iconvValid != meValid) {
            if(randFlipMe != randFlipIconv) {
                printf("Ignore this: ");
            }
            printf("iconv = %s me = %s for 0x%02x%02x%02x%02x, length = %d\n",
                iconvValid? "true" : "false", meValid? "true" : "false",
                inbuf[0], inbuf[1], inbuf[2], inbuf[3], length);
            fflush(stdout);
        }
        value++;
        if((value & 0xffffff) == 0) {
            printf("checked through 0x%x\n", value);
            fflush(stdout);
        }
    } while(value != 0);
    return 0;
}
