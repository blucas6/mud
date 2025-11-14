CC		  := gcc
CFLAGS  := -Wall -Wextra
LDFLAGS := -lncurses
TARGETS := main

all: $(TARGETS)

main: main.c
	$(CC) $(CFLAGS) -o mud.out $<

clean:
	rm -f $(TARGETS)
