CC=gcc
CFLAGS=-Wall -Wno-parentheses -std=gnu99 -L/usr/local/lib -L/usr/lib
LDFLAGS=-ldl -ltcc

all: bibgen byteplay clive

bibgen: bibgen.c
	$(CC) $(CFLAGS) -o bibgen bibgen.c $(LDFLAGS)

byteplay: byteplay.c
	$(CC) $(CFLAGS) -o byteplay byteplay.c $(LDFLAGS)

clive: clive.c
	$(CC) $(CFLAGS) -o clive clive.c $(LDFLAGS)

clean:
	rm -rf bibgen byteplay clive
