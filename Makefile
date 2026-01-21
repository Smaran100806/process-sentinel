CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/proc_parser.c src/detector.c src/logger.c src/utils.c
OBJ = $(SRC:.c=.o)
TARGET = sentinel

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

clean:
	rm -f src/*.o $(TARGET)