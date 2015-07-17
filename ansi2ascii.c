// This program simply converts ANSI to ASCII, making sensical conversions
// suitable for casual reading.

#include <stdio.h>
#include <stdbool.h>

static int prevWasSpace = false;
static int needSpace = false;
static char wordBuf[42];
static int pos;

// Add a character to the word buffer.
static void addchar(char c) {
    wordBuf[pos++] = c;
    wordBuf[pos] = '\0';
}

// Add the string to the word buffer, making sure to surround it with spaces.
static inline void addWord(char *string) {
    if(!prevWasSpace) {
        addchar(' ');
    }
    char *p = string;
    while(*p != '\0') {
        addchar(*p++);
    }
    prevWasSpace = false;
    needSpace = true;
}

// Replace the character with an ASCII equivalent, which can be an equivalent
// character, a word, or nothing.
static inline void addEquivalent(unsigned char c) {
    switch(c) {
    case 0x7f: break;
    case 0x80: addWord("euro"); break;
    case 0x81: break;
    case 0x82: addchar('\''); break;
    case 0x83: addchar('f'); break;
    case 0x84: addchar('"'); break;
    case 0x85: addWord("..."); break;
    case 0x86: break;
    case 0x87: break;
    case 0x88: addchar('^'); break;
    case 0x89: addchar('%'); break;
    case 0x8A: addchar('S'); break;
    case 0x8B: addchar('<'); break;
    case 0x8C: addchar('E'); break;
    case 0x8D: break;
    case 0x8E: addchar('Z'); break;
    case 0x8F: break;
    case 0x90: break;
    case 0x91: addchar('\''); break;
    case 0x92: addchar('\''); break;
    case 0x93: addchar('"'); break;
    case 0x94: addchar('"'); break;
    case 0x95: addchar('*'); break;
    case 0x96: addchar('-'); break;
    case 0x97: addchar('-'); break;
    case 0x98: addchar('~'); break;
    case 0x99: addWord("TM"); break;
    case 0x9A: addchar('S'); break;
    case 0x9B: addchar('>'); break;
    case 0x9C: addchar('e'); break;
    case 0x9D: break;
    case 0x9E: addchar('z'); break;
    case 0x9F: addchar('Y'); break;
    case 0xA0: addchar(' '); break;
    case 0xA1: break;
    case 0xA2: addWord("cents"); break;
    case 0xA3: addWord("pounds"); break;
    case 0xA4: addWord("currency"); break;
    case 0xA5: addWord("yen"); break;
    case 0xA6: addchar('|'); break;
    case 0xA7: break;
    case 0xA8: break;
    case 0xA9: addWord("copyright"); break;
    case 0xAA: addchar('a'); break;
    case 0xAB: addWord("<<"); break;
    case 0xAC: addWord("not"); break;
    case 0xAD: addchar('-'); break;
    case 0xAE: addWord("restricted"); break;
    case 0xAF: break;
    case 0xB0: addWord("degrees"); break;
    case 0xB1: addWord("plus/minus"); break;
    case 0xB2: addchar('2'); break;
    case 0xB3: addchar('3'); break;
    case 0xB4: addchar('\''); break;
    case 0xB5: addchar('u'); break;
    case 0xB6: break;
    case 0xB7: addchar('*'); break;
    case 0xB8: addchar(','); break;
    case 0xB9: addchar('1'); break;
    case 0xBA: addchar('0'); break;
    case 0xBB: addWord(">>"); break;
    case 0xBC: addWord("1/4"); break;
    case 0xBD: addWord("1/3"); break;
    case 0xBE: addWord("3/4"); break;
    case 0xBF: break;
    case 0xC0: addchar('A'); break;
    case 0xC1: addchar('A'); break;
    case 0xC2: addchar('A'); break;
    case 0xC3: addchar('A'); break;
    case 0xC4: addchar('A'); break;
    case 0xC5: addchar('A'); break;
    case 0xC6: addWord("AE"); break;
    case 0xC7: addchar('C'); break;
    case 0xC8: addchar('E'); break;
    case 0xC9: addchar('E'); break;
    case 0xCA: addchar('E'); break;
    case 0xCB: addchar('E'); break;
    case 0xCC: addchar('I'); break;
    case 0xCD: addchar('I'); break;
    case 0xCE: addchar('I'); break;
    case 0xCF: addchar('I'); break;
    case 0xD0: addchar('D'); break;
    case 0xD1: addchar('N'); break;
    case 0xD2: addchar('O'); break;
    case 0xD3: addchar('O'); break;
    case 0xD4: addchar('O'); break;
    case 0xD5: addchar('O'); break;
    case 0xD6: addchar('O'); break;
    case 0xD7: addchar('*'); break;
    case 0xD8: addchar('0'); break;
    case 0xD9: addchar('U'); break;
    case 0xDA: addchar('U'); break;
    case 0xDB: addchar('U'); break;
    case 0xDC: addchar('U'); break;
    case 0xDD: addchar('Y'); break;
    case 0xDE: addchar('Y'); break;
    case 0xDF: addchar('s'); break;
    case 0xE0: addchar('a'); break;
    case 0xE1: addchar('a'); break;
    case 0xE2: addchar('a'); break;
    case 0xE3: addchar('a'); break;
    case 0xE4: addchar('a'); break;
    case 0xE5: addchar('a'); break;
    case 0xE6: addWord("ae"); break;
    case 0xE7: addchar('c'); break;
    case 0xE8: addchar('e'); break;
    case 0xE9: addchar('e'); break;
    case 0xEA: addchar('e'); break;
    case 0xEB: addchar('e'); break;
    case 0xEC: addchar('i'); break;
    case 0xED: addchar('i'); break;
    case 0xEE: addchar('i'); break;
    case 0xEF: addchar('i'); break;
    case 0xF0: addchar('o'); break;
    case 0xF1: addchar('n'); break;
    case 0xF2: addchar('o'); break;
    case 0xF3: addchar('o'); break;
    case 0xF4: addchar('o'); break;
    case 0xF5: addchar('o'); break;
    case 0xF6: addchar('o'); break;
    case 0xF7: addchar('/'); break;
    case 0xF8: addchar('0'); break;
    case 0xF9: addchar('u'); break;
    case 0xFA: addchar('u'); break;
    case 0xFB: addchar('u'); break;
    case 0xFC: addchar('u'); break;
    case 0xFD: addchar('y'); break;
    case 0xFE: addchar('y'); break;
    case 0xFF: addchar('y'); break;
    default:
        fprintf(stderr, "Unexpected character %02x\n", (unsigned char)c);
    }
}

// Convert an ANSI character to ASCII.  The returned string is zero-terminated.
char *swConvertANSIToASCII(char c) {
    // Print a space if we need one to separate from a word we printed
    pos = 0;
    if(c > ' ' && needSpace) {
        addchar(' ');
    }
    needSpace = false;
    if((unsigned char)c < 0x7f) {
        addchar(c);
    } else {
        addEquivalent(c);
    }
    return wordBuf;
}

