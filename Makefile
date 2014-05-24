#CFLAGS=-Wall -O2 -fPIC -m32
CFLAGS=-Wall --std=c99
CC=gcc

ESPEAK=engines/sh_espeak
IBMTTS=engines/sh_ibmtts
PICO=engines/sh_picotts
EXAMPLE=engines/sh_example

ENGINES=$(ESPEAK) $(IBMTTS) $(PICO) $(EXAMPLE)

all: engines $(ENGINES)

$(EXAMPLE): server.c example_engine.c engine.h
	$(CC) -O2 -I . -o $(EXAMPLE) example_engine.c server.c -lespeak
 
$(ESPEAK): server.c espeak_engine.c engine.h
	$(CC) $(CFLAGS) -O2 -o $(ESPEAK) server.c espeak_engine.c -lespeak -lm -lpthread

# Note that this cannot be compiled with -O2 due to unknown bugs.
$(IBMTTS): server.c ibmtts_engine.c engine.h
	$(CC) $(CFLAGS) -m32 -I/opt/IBM/ibmtts/inc -o $(IBMTTS) server.c ibmtts_engine.c -libmeci

$(PICO): pico_engine.c server.c engine.h
	gcc -Wall -O2 -o $(PICO) pico_engine.c server.c -lttspico -lpopt -lm

engines:
	mkdir engines

clean:
	rm -f $(ENGINES)
