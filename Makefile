CC = gcc
CXX = g++

CFLAGS = -Wall -Wextra -pedantic -std=c99
CXXFLAGS = -Wall -Wextra -std=gnu11

DEBUG = -g
OPTIMIZE = -O3

len: len.c
	$(CC) $(CFLAGS) $(OPTIMIZE) $^ -o $@

len-debug: len.c
	$(CC) $(CFLAGS) $(OPTIMIZE) ${DEBUG} $^ -o $@
