# Makefile for simpledb tutorial

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
TARGET = db
SRC = db.c
DB ?= test.db   # default database name, override with `make run DB=mydb.db`

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET) $(DB)

clean:
	rm -f $(TARGET) $(DB)
