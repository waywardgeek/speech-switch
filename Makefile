#CFLAGS=-Wall -O3 --std=c99 -D_POSIX_SOURCE
CFLAGS=-Wall -g --std=c99 -D_POSIX_SOURCE
#CC=gcc
CC=gcc
PREFIX=/usr/local

ESPEAK=libexec/speechsw/espeak/sw_espeak
IBMTTS=libexec/speechsw/ibmtts/sw_ibmtts
PICOTTS=libexec/speechsw/picotts/sw_picotts
EXAMPLE=sw_example

# You must manually build the supported speech synths on your system first.
# Update these paths to reflect the insteallation on your system.  We do this so
# that modules under libexec/speechsw are binary-compatible across Linux distros.

# ESPEAK-NG
# Source downloaded with 'git clone https://github.com/espeak-ng/espeak-ng.git'
# git commit: a4c2ea83c0c072e1227fc7e751ea58a066b52712
# Configure espeak-ng with: configure --without-pcaudiolib --without-speechplayer
ESPEAK_LIB=../espeak-ng/src/.libs/libespeak-ng.a
ESPEAK_DATA=../espeak-ng/espeak-ng-data

# IBMTTS
# Install Voxin from http://oralux.org, with the language of your choice.
# This is version 2.2.  Only English has been tested so far.
IBMTTS_LIB=/opt/oralux/voxin/lib/libvoxin.so
IBMTTS_DATA=/opt/IBM/ibmtts/voicedata/

# PICOTTS
# Source downloaded with 'git clone https://github.com/naggety/picotts.git'
# git commit: e3ba46009ee868911fa0b53db672a55f9cc13b1c
# Configure picotts in the pico subdirectory with './configure'.
PICOTTS_LIB=../picotts/pico/.libs/libttspico.a
PICOTTS_DATA=../picotts/pico/lang

ENGINES=$(ESPEAK) $(IBMTTS) $(PICOTTS) $(EXAMPLE)

all: $(ENGINES) bin/sw-say lib/libspeechsw.so

$(EXAMPLE): engine.c example_engine.c util.c engine.h
	$(CC) -O2 -I . -o $(EXAMPLE) example_engine.c engine.c util.c -lespeak
 
$(ESPEAK): engine.c espeak_engine.c util.c engine.h
	mkdir -p $(dir $(ESPEAK))
	$(CC) $(CFLAGS) -O2 -o $(ESPEAK) engine.c util.c espeak_engine.c $(ESPEAK_LIB) -lm -pthread
	cp -r $(ESPEAK_DATA) $(dir $(ESPEAK))

# Note that this cannot be compiled with -O2 due to unknown bugs.
$(IBMTTS): engine.c ibmtts_engine.c util.c engine.h
	mkdir -p $(dir $(IBMTTS))
	$(CC) $(CFLAGS) -I/opt/IBM/ibmtts/inc -o $(IBMTTS) engine.c util.c ibmtts_engine.c $(IBMTTS_LIB)
	cp -r $(IBMTTS_DATA) $(dir $(IBMTTS))

$(PICOTTS): pico_engine.c engine.c util.c engine.h
	mkdir -p $(dir $(PICOTTS))
	$(CC) $(CFLAGS) -o $(PICOTTS) pico_engine.c engine.c util.c $(PICOTTS_LIB) -lpopt -lm
	cp -r $(PICOTTS_DATA) $(dir $(PICOTTS))

bin/sw-say: sw-say.c speechsw.c speechsw.h ansi2ascii.c util.c util.h wave.c wave.h
	mkdir -p bin
	$(CC) $(CFLAGS) -o bin/sw-say sw-say.c speechsw.c ansi2ascii.c util.c wave.c ../sonic/libsonic.a -lm

lib/libspeechsw.so: speechsw.c speechsw.h util.c util.h
	mkdir -p lib
	$(CC) -c -fpic $(CFLAGS) speechsw.c util.c
	gcc -shared -o lib/libspeechsw.so speechsw.o util.o ../sonic/libsonic.a

install: all
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/libexec
	cp -r libexec/speechsw $(PREFIX)/libexec
	install bin/sw-say $(PREFIX)/bin
	mkdir -p $(PREFIX)/include/speechsw
	cp util.h speechsw.h $(PREFIX)/include/speechsw
	cp lib/libspeechsw.so $(PREFIX)/lib

uninstall:
	rm -rf $(PREFIX)/libexec/speechsw
	rm -rf $(PREFIX)/include/speechsw
	rm -f $(PREFIX)/lib/libspeechsw.so
	rm -f $(PREFIX)/bin/sw-say

clean:
	rm -r bin lib libexec
