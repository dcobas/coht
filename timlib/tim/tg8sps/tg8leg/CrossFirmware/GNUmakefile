
#
# Makefile to produce code for the tg8
#

CC   = /ps/local/$(OS)/m68k/m68k--aout/bin/gcc -m68020 -save-temps
LD   = /ps/local/$(OS)/m68k/m68k--aout/bin/ld -s -v -n
CV   = $(CF)/Yacc/tg8pars
X    = ../firmware
F    = ../firmware
CF   = ../CrossFirmware

# Compilation flags
# -----------------

CFLAGS = -v -nostdinc -I. -I$(X) -I/tmp -mnobitfield -ffixed-a6 \
	 -fstrength-reduce -fexpensive-optimizations -DCROSS_COMPILER=1

#Switch these on when making tg8.asm for mixed listing: -fverbose-asm -g

all: tg8.exe

tg8.exe: tg8.o
	$(LD) -Ttext 0x4000 -Tdata 0x7E00 -Tbss 0x7E00 -m m68kaout -M -o tg8.exe tg8.o >tg8.exe.m
	grep _e tg8.exe.m
	grep HrInt /tmp/tg8Sym.c
	ls -l tg8.exe*

tg8.o:  $(X)/tg8.c $(X)/mbx.c $(X)/isr.c $(X)/immproc.c \
	$(X)/tg8.h $(X)/tg8P.h $(X)/tg8Priv.h $(X)/macros.h $(X)/tg8Pub.h $(X)/tg8Hardw.h
	cd ../firmware; $(CV) <tg8Sym.h asm >/tmp/tg8Sym.c
	cd ../firmware; $(CC) -o $(CF)/tg8.o -c $(CFLAGS) $(X)/tg8.c
	cd ../firmware; mv tg8.s $(CF)/.; mv tg8.i $(CF)/.

tg8.asm: tg8.s
	# .text
	# .globl __main
	# . = 0x4000
	# __main: bra Main
	# To get correct addresses add .= 0x4000 after main entry point in tg8.s
	/ps/local/$(OS)/m68k/m68k--aout/bin/as -m68020 -o ../CrossFirmware/tg8.o tg8.s -a > tg8.asm

#eof
