# Speech Switch

The goal of Speech Switch is making Linux a hospitable place for commercial
text-to-speech engines like Voxin.  This will benifit many blind and low vision
Linux users who, like me, rely on Orca every day to be productive.

Speech Switch will be an alternative to Speech Dispatcher, and is designed from
the ground up to make TTS engine binaries portable between distros, just like
applications on Windows just keep running year after year.

This will finally enable Linux to be a unified market for text-to-speech
vendors.  As a result, Linux will become more attractive to blind and low vision
users.  Many of us _require_ Eloquence, which in Linux is called Voxin.  Espeak
is universally available, which is awesome, but my own productivity is
dramatically improved when using Voxin.

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
lib/speechsw should run on any 64-bit Linux distro.

On Ubuntu, I install the base libraries with:

$ sudo apt-get install libespeak-dev libttspico-dev libsonic-dev
