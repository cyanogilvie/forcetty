DESTDIR=
PREFIX=/usr/local
#CC = clang
#LD = ld.lld
CFLAGS_OPTIMIZE = -O3
CFLAGS_DEBUG = -Og
CFLAGS = $(CFLAGS_OPTIMIZE) -gdwarf-5 -Wall -Werror -std=c2x
LDFLAGS = -lutil

all: forcetty

forcetty: forcetty.o
	$(CC) $(LDFLAGS) $(CFLAGS) -fwhole-program -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f core *.o forcetty

install: forcetty
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp forcetty $(DESTDIR)$(PREFIX)/bin/

.PHONY: all clean
