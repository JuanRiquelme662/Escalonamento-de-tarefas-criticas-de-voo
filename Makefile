CC = gcc
CFLAGS = -Wall -Wextra
TARGET = scheduler
SRC = main.c funcoes.c

$(TARGET): $(SRC) funcoes.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) *.out