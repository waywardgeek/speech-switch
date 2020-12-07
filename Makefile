#CFLAGS=-Wall -O3 --std=c99 -D_POSIX_SOURCE
CFLAGS=-Wall -g --std=c99 -D_POSIX_SOURCE
#CC=gcc
CC=gcc

ESPEAK=lib/speechsw/engines/sw_espeak
IBMTTS=lib/speechsw/engines/sw_ibmtts
PICO=lib/speechsw/engines/sw_picotts
EXAMPLE=sw_example

ENGINES=$(ESPEAK) $(IBMTTS) $(PICO) $(EXAMPLE)

all: bin lib/speechsw/engines $(ENGINES) bin/sw-say bin/speechsw

$(EXAMPLE): server.c example_engine.c util.c engine.h
	$(CC) -O2 -I . -o $(EXAMPLE) example_engine.c server.c util.c -lespeak
 
$(ESPEAK): server.c espeak_engine.c util.c engine.h
	$(CC) $(CFLAGS) -O2 -o $(ESPEAK) server.c util.c espeak_engine.c -lespeak -lm -pthread

# Note that this cannot be compiled with -O2 due to unknown bugs.
$(IBMTTS): server.c ibmtts_engine.c util.c engine.h
	$(CC) $(CFLAGS) -I/opt/IBM/ibmtts/inc -o $(IBMTTS) server.c util.c ibmtts_engine.c -libmeci

$(PICO): pico_engine.c server.c util.c engine.h
	gcc $(CFLAGS) -o $(PICO) pico_engine.c server.c util.c -lttspico -lpopt -lm

bin/sw-say: sw-say.c speechsw.c ansi2ascii.c util.c wave.c engine.h speechsw.h
	gcc $(CFLAGS) -o bin/sw-say sw-say.c speechsw.c ansi2ascii.c util.c wave.c -lsonic -lm

bin/speechsw: speechsw.c speechsw.c ansi2ascii.c util.c wave.c engine.h speechsw.h
	gcc $(CFLAGS) -o bin/speechsw sw-say.c speechsw.c ansi2ascii.c util.c wave.c -lsonic -lm

bin:
	mkdir bin

lib/speechsw/engines:
	mkdir -p lib/speechsw/engines

install: all
	install -d /usr/bin /usr/lib/speechsw/engines
	install bin/sw-say /usr/bin
	install lib/speechsw/engines/* /usr/lib/speechsw/engines

clean:
	rm -f $(ENGINES) bin/sw-say
