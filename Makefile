CC = gcc
CFLAGS = -Wall -Wextra
OUT = musin
TARGET = main.c

main: $(TARGET)
	$(CC) $(CFLAGS) -o $(OUT) $(TARGET)