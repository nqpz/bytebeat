CC=gcc
CFLAGS=-Wall -Wno-parentheses -std=gnu99 $(shell pkg-config --cflags libtcc)
LDFLAGS=$(shell pkg-config --libs libtcc)

all: bibgen byteplay clive

bibgen: bibgen.c
	$(CC) $(CFLAGS) -o bibgen bibgen.c $(LDFLAGS)

byteplay: byteplay.c
	$(CC) $(CFLAGS) -o byteplay byteplay.c $(LDFLAGS)

clive: clive.c
	$(CC) $(CFLAGS) -o clive clive.c $(LDFLAGS)

clean:
	rm -rf bibgen byteplay clive
