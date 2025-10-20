NAME=dirSelect
SRC=./src/main.c
CC=gcc
CFLAGS=-I . -Wall -lncurses

.PHONY: all
.PHONY: build
.PHONY: clean

all: build

build:
	$(CC) $(CFLAGS) $(SRC) -o $(NAME)
	$(CC) -O3 -Wall ./src/fastread.c -o read
	
clean:
	rm -f $(NAME)
	rm -f read
