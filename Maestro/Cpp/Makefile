# Makefile
CC=g++
CFLAGS=-I.
# Flasgs needed for miniaudio
CFLAGS+=-msse2 -mavx2 -ldl -lpthread -lm
# Flags needed for libopus
# CFLAGS+=-lopusfile -I/usr/include/opus
SRCS=$(wildcard lib/*.cpp main.cpp)
OBJS=$(addprefix build/, $(notdir $(SRCS:.cpp=.o)))

all: build/main

debug: CFLAGS+=-DDEBUG -g
debug: all

opti: CFLAGS+=-O3
opti: all

build/main: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

build/%.o: lib/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

build/%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f build/*.o build/main

.PHONY: all clean