CC=gcc
CFLAGS=-Wall `pkg-config --cflags gtk+-3.0`
LDFLAGS=`pkg-config --libs gtk+-3.0` -lm -lpthread
SRC=main.c shell_functions.c callbacks.c utils.c auto_suggest.c voice_recognition.c kernel_features.c command_suggestions.c
BIN=main

all: $(BIN)

$(BIN): $(SRC)
$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LDFLAGS)

run: $(BIN)
./$(BIN)

clean:
rm -f $(BIN)

.PHONY: all run clean
