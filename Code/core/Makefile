# Makefile

## Compile options
# use the clang compiler
CC=clang
# Wall: Show all possible warnings
# pedantic: Be very strict about warnings, show even more than with -ansi flag
# Werror: Make all warnings to errors
# g: Produce debug information which are needed for debugging the programme
CFLAGS=-Wall -pedantic -Werror -g -O2
LDFLAGS=-pthread -lrt -I/usr/local/include -L/usr/local/lib -lwiringPi
# Postprocessor option to tell the compiler to look in the current directory
# for dependencies. Needed to the "depend" rule.
CPPFLAGS=-I.

# Command variables
RM = rm -f

# Help variable
TARGET = robot
OBJECTS = logger.o time_utils.o priority.o gpiosetup.o
SOURCES = $(TARGET).c $(OBJECTS:.o=.c)
HEADERS = $(OBJECTS:.o=.h)

# Pattern rules
%:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

# Standard targets
.PHONY: all clean depend

all:$(TARGET) $(TARGET1)

# Clean up everything which was created by the makefile
clean:
	$(RM) $(TARGET) $(TARGET1) $(TARGET).o $(OBJECTS) depend

depend:$(SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) -MM $(SOURCES) > $@

# Targets for programme creation
$(TARGET):$(TARGET).o $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

## Build robot binary
#$(TARGET1): main.c priority.c gpiosetup.c time_utils.c
#	$(CC) -g $(LDFLAGS) $^ -o $@

# Include dependencies
include depend
