CC = gcc
CFLAGS = -Wall -Wextra
SRCDIR = src
TARGET = musin
SRC = $(SRCDIR)/main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)