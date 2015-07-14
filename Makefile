#CFLAGS=-Wall -O2 -fPIC -m32
CFLAGS=-Wall --std=c99 -D_POSIX_SOURCE
#CC=gcc
CC=gcc

ESPEAK=engines/sw_espeak
IBMTTS=engines/sw_ibmtts
PICO=engines/sw_picotts
EXAMPLE=engines/sw_example

ENGINES=$(ESPEAK) $(IBMTTS) $(PICO) $(EXAMPLE)

all: engines $(ENGINES) sw-say

$(EXAMPLE): server.c example_engine.c util.c engine.h
	$(CC) -O2 -I . -o $(EXAMPLE) example_engine.c server.c util.c -lespeak
 
$(ESPEAK): server.c espeak_engine.c util.c engine.h
	$(CC) $(CFLAGS) -O2 -o $(ESPEAK) server.c util.c espeak_engine.c -lespeak -lm -pthread

# Note that this cannot be compiled with -O2 due to unknown bugs.
$(IBMTTS): server.c ibmtts_engine.c util.c engine.h
	$(CC) $(CFLAGS) -m32 -I/opt/IBM/ibmtts/inc -o $(IBMTTS) server.c util.c ibmtts_engine.c -libmeci

$(PICO): pico_engine.c server.c util.c engine.h
	gcc -Wall -O2 -o $(PICO) pico_engine.c server.c util.c -lttspico -lpopt -lm

sw-say: sw-say.c client.c util.c wave.c engine.h speechswitch.h
	gcc -Wall -O2 -o sw-say sw-say.c client.c util.c wave.c

swdatabase.c: swdatabase.h

swdatabase.h: SpeechSwitch.dd
	datadraw SpeechSwitch.dd

engines:
	mkdir engines

clean:
	rm -f $(ENGINES) sw-say
