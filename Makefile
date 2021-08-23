SHELL = sh

CC ?= gcc
AR ?= ar
CP ?= cp
RM ?= rm
STRIP ?= strip
MKDIR ?= mkdir

ARFLAGS ?= rcs

PREFIX ?= /usr/local

DESTDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include

CFLAGS ?= -Ofast -g3 -Wextra

LIBOBJ = tex.o

all: ctextoppm

ctextoppm: libctex.a
	$(CC) $(CFLAGS) -Wall ctextoppm.c -octextoppm -L. -I. -lnetpbm -lctex

tex.o:
	$(CC) $(CFLAGS) -Wall -c libctex/tex.c -otex.o

libctex.a: $(LIBOBJ)
	$(AR) $(ARFLAGS) libctex.a $(LIBOBJ)

install: libctex.a ctextoppm
	$(CP) libctex.a $(LIBDIR)
	$(MKDIR) $(INCLUDEDIR)/libctex || true
	$(CP) libctex/tex.h $(INCLUDEDIR)/libctex
	$(CP) ctextoppm $(DESTDIR)

clean:
	$(RM) *.o *.a ctextoppm
