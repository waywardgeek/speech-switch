#CFLAGS=-Wall -O3 --std=c99 -D_POSIX_SOURCE
CFLAGS=-Wall -g --std=c99 -D_POSIX_SOURCE
#CC=gcc
CC=gcc

ESPEAK=lib/speechsw/engines/sw_espeak
IBMTTS=lib/speechsw/engines/sw_ibmtts
PICO=lib/speechsw/engines/sw_picotts
EXAMPLE=sw_example

# You must manually build the supported speech synths on your system first.
# Update these paths to reflect the insteallation on your system.  We do this so
# that modules under lib/speechsw are binary-compatible across Linux distros.

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

# PICO
# Source downloaded with 'git clone https://github.com/naggety/picotts.git'
# git commit: e3ba46009ee868911fa0b53db672a55f9cc13b1c
# Configure picotts in the pico subdirectory with './configure'.
PICO_LIB=../picotts/pico/.libs/libttspico.a
PICO_DATA=../picotts/pico/lang

ENGINES=$(ESPEAK) $(IBMTTS) $(PICO) $(EXAMPLE)

all: $(ENGINES) bin/sw-say bin/speechsw

$(EXAMPLE): server.c example_engine.c util.c engine.h lib/speechsw/engines
	$(CC) -O2 -I . -o $(EXAMPLE) example_engine.c server.c util.c -lespeak
 
$(ESPEAK): server.c espeak_engine.c util.c engine.h lib/speechsw/engines
	$(CC) $(CFLAGS) -O2 -o $(ESPEAK) server.c util.c espeak_engine.c $(ESPEAK_LIB) -lm -pthread
	cp -r $(ESPEAK_DATA) lib/speechsw/data

# Note that this cannot be compiled with -O2 due to unknown bugs.
$(IBMTTS): server.c ibmtts_engine.c util.c engine.h lib/speechsw/engines
	$(CC) $(CFLAGS) -I/opt/IBM/ibmtts/inc -o $(IBMTTS) server.c util.c ibmtts_engine.c $(IBMTTS_LIB)
	cp -r $(IBMTTS_DATA) lib/speechsw/data

$(PICO): pico_engine.c server.c util.c engine.h lib/speechsw/engines
	gcc $(CFLAGS) -o $(PICO) pico_engine.c server.c util.c $(PICO_LIB) -lpopt -lm
	cp -r $(PICO_DATA) lib/speechsw/data

bin/sw-say: bin sw-say.c speechsw.c ansi2ascii.c util.c wave.c engine.h speechsw.h
	gcc $(CFLAGS) -o bin/sw-say sw-say.c speechsw.c ansi2ascii.c util.c wave.c -lsonic -lm

bin/speechsw: speechsw.c speechsw.c ansi2ascii.c util.c wave.c engine.h speechsw.h
	gcc $(CFLAGS) -o bin/speechsw sw-say.c speechsw.c ansi2ascii.c util.c wave.c -lsonic -lm

bin:
	mkdir bin

lib/speechsw/engines: lib/speechsw/data
	mkdir -p lib/speechsw/engines

lib/speechsw/data:
	mkdir -p lib/speechsw/data

install: all
	install -d /usr/bin /usr/lib/speechsw/engines
	install bin/sw-say /usr/bin
	install lib/speechsw/engines/* /usr/lib/speechsw/engines

clean:
	rm -r bin lib
