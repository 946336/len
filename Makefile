# Makefile for len

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

DEBUG = -g
OPTIMIZE = -O3

len: len.c
	$(CC) $(CFLAGS) $(OPTIMIZE) $^ -o $@

len-debug: len.c
	$(CC) $(CFLAGS) $(DEBUG) $^ -o $@

configure:
	sed -i "s|PATH_TO_EXECUTABLE|`pwd`|g" lenfuncs.py lengui.py
	ln -sf lengui.py glen

install:
	echo "Nothing to be done. See Makefile if you're curious."

make clean:
	rm -f len len-debug