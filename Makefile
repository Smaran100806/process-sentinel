CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/proc_parser.c src/detector.c src/logger.c src/utils.c
OBJ = $(SRC:.c=.o)
DEPS = include/proc_parser.h include/detector.h include/logger.h
TARGET = sentinel

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f src/*.o $(TARGET)