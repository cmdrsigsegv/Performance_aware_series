CC = gcc
CFLAGS = -Wall -Wextra
TARGET = run

SRCS := $(wildcard *.c)

.PHONY: all clean

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)