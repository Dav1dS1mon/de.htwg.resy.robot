# Makefile
# @author: Torben Woltjen, Dennis Grießer, David Simon
# @date: 14.04.2015

# Define a compiler and set options for the compiler and linker
CC=gcc
CFLAGS=-W -Wall -Wno-comment -std=c99 -pedantic
# Allow "//"-comments (also known as C++-style-comments by removing the corresponding warning
#CFLAGS+=-Wno-comment
LDFLAGS=-pthread -lrt -I/usr/local/include -L/usr/local/lib -lwiringPi
RM=rm -f

# Variables
TARGET=distance_test motor_test sensor_motor

#PHONY REGELN
.PHONY: all clean

all: $(TARGET)

clean:
	$(RM) $(TARGET)

distance_test: distance_test.c
	$(CC) -g $(LDFLAGS) $^ -o $@

sensor_motor: sensor_motor.c
	$(CC) -g $(LDFLAGS) $^ -o $@



