# *-* Makefile *-*

include=-I ../include

CC=gcc
CFLAGS=$(include) -D DEBUG

SOURCES := $(wildcard *.c)

OBJECTS := $(patsubst %.c,../obj/%.o,$(SOURCES))

default: $(OBJECTS)

../obj/%.o: %.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(OBJECTS:.o=.d)