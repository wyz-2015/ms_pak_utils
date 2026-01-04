CC=gcc
SRC_C=deque.c reader.c main.c common.c writer.c
SRC_H=args.h common.h deque.h pak.h reader.h writer.h
INCLUDE=-I./
BIN=./../bin/msxx_pak
LIBS=
CFLAGS=

ifeq ($(OS),Windows_NT)
	BIN+=.exe
	LIBS+=-largp
endif

all : MSXX_PAK

MSXX_PAK : $(BIN)

$(BIN) : $(SRC_C) $(SRC_H)
	$(CC) $(SRC_C) $(INCLUDE) $(LIBS) $(CFLAGS) -o $(BIN)
