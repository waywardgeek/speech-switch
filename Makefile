#CFLAGS=-Wall -O3 --std=c99 -D_POSIX_SOURCE
CFLAGS=-Wall -g --std=c99 -D_POSIX_SOURCE
#CC=gcc
CC=gcc

ESPEAK=lib/speechswitch/engines/sw_espeak
IBMTTS=lib/speechswitch/engines/sw_ibmtts
PICO=lib/speechswitch/engines/sw_picotts
EXAMPLE=sw_example

ENGINES=$(ESPEAK) $(IBMTTS) $(PICO) $(EXAMPLE)

all: bin lib/speechswitch/engines $(ENGINES) bin/sw-say

$(EXAMPLE): server.c example_engine.c util.c engine.h
	$(CC) -O2 -I . -o $(EXAMPLE) example_engine.c server.c util.c -lespeak
 
$(ESPEAK): server.c espeak_engine.c util.c engine.h
	$(CC) $(CFLAGS) -O2 -o $(ESPEAK) server.c util.c espeak_engine.c -lespeak -lm -pthread

# Note that this cannot be compiled with -O2 due to unknown bugs.
$(IBMTTS): server.c ibmtts_engine.c util.c engine.h
	$(CC) $(CFLAGS) -m32 -I/opt/IBM/ibmtts/inc -o $(IBMTTS) server.c util.c ibmtts_engine.c -libmeci

$(PICO): pico_engine.c server.c util.c engine.h
	gcc $(CFLAGS) -o $(PICO) pico_engine.c server.c util.c -lttspico -lpopt -lm

bin/sw-say: sw-say.c client.c ansi2ascii.c util.c wave.c engine.h speechswitch.h
	gcc $(CFLAGS) -o bin/sw-say sw-say.c client.c ansi2ascii.c util.c wave.c -lsonic -lm

swdatabase.c: swdatabase.h

swdatabase.h: SpeechSwitch.dd
	datadraw SpeechSwitch.dd

bin:
	mkdir bin

lib/speechswitch/engines:
	mkdir -p lib/speechswitch/engines

install: all
	install -d /usr/bin /usr/lib/speechswitch/engines
	install bin/sw-say /usr/bin
	install lib/speechswitch/engines/* /usr/lib/speechswitch/engines

clean:
	rm -f $(ENGINES) bin/sw-say
