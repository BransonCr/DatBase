# Makefile for simpledb tutorial

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
TARGET = db
SRC = db.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)