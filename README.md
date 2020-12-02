# Speech Switch

Making Linux a hospitable place for commercial TTS engines like Voxin.

# License

The initial files in this directory were written by me, Bill Cox in 2011.  I
place all of this code in the public domain.  Feel free to do with it as you
wish.  If you have questions, I can be contacted as waywardgeek@gmail.com.

I also provide this software under the Apache 2.0 license, for anyone who
requires that software have a license.  Since I also place this code into the
public domain, feel free to offer it to others under any license you like.

I wrote ibmtts_engine.c, in 2011, and would put it in the public domain, except
that some code in ibmtts_engine.c was copied from speech-dispatcher's ibmtts.c
file.  Therefore, I release ibmtts_engine.c under the LGPL v2.1 license, as
required by the terms of the license in ibmtts.c.  If you want to write a new
engine, and do not want to release source code as LGPL requires, please use the
espeak_engine.c file as your starting point.  The functions you need to
implement are documented in engine.h.

# Building in Ubuntu or Debian

To compile the engines in Ubuntu or Debian, first install build dependencies for
libttspico, espeak, and the Sonic speech speedup library.  Preferably, you will
also have a valid license of voxin, so you can build the ibmtts engine.  To
compile all the linux engines, type 'make'.  The resulting executables in
lib/speechswitch should run on any 64-bit Linux distro.

On Ubuntu, I install the base libraries with:

$ sudo apt-get install libespeak-dev libttspico-dev libsonic-dev
