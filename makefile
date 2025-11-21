CC		  := gcc
CFLAGS  := -Wall -Wextra $(shell pkg-config --cflags libxml-2.0)
LDFLAGS := $(shell pkg-config --libs libxml-2.0)
TARGETS := main

all: $(TARGETS)

main: main.c
	$(CC) $(CFLAGS) -o mud.out $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)
