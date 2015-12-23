# Makefile for len

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

DEBUG = -g
OPTIMIZE = -O3

len: len.c
	$(CC) $(CFLAGS) $(OPTIMIZE) $^ -o $@

len-debug: len.c
	$(CC) $(CFLAGS) $(DEBUG) $^ -o $@

make clean:
	rm -f len len-debug