CC = @PSP_CC@
CCAS = $(CC)
AR = @PSP_AR@
RANLIB = @PSP_RANLIB@
CFLAGS = 

EXEEXT =
OBJEXT = o

AM_CFLAGS = -I $(top_srcdir)/src/include -I $(top_srcdir)/src -I $(top_srcdir)/src/audiocore

if OPTIMIZE
AM_CFLAGS += -Os -G0 -Wall -fno-builtin 
else
AM_CFLAGS += -O0 -G0 -g -Wall -fno-builtin
endif
CCASFLAGS = $(CFLAGS) $(INCLUDES)

LDFLAGS = -mno-crt0 -nostartfiles
	
noinst_PROGRAMS= dummy.elf

dummy_elf_SOURCES = main.c imports.S

TARGET = dummy

PSP_FW_VERSION = 639
BUILD_PRX=1

PSPSDK=$(shell psp-config --pspsdk-path)
libdir = $(top_srcdir)/lib
LIBS=-L$(libdir) -lpspkernel -lpspsystemctrl_kernel

include $(PSPSDK)/lib/build.mak

all: 
