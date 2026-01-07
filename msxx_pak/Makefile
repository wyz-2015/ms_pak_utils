CC=gcc
SRC_C=deque.c reader.c main.c common.c writer.c pak.c
SRC_H=args.h common.h deque.h pak.h reader.h writer.h
INCLUDE=-I./
BIN=./../bin/msxx_pak
LIBS=

ifeq ($(OS),Windows_NT)
	BIN:=$(BIN).exe
	LIBS+=-largp
endif

all : MSXX_PAK

MSXX_PAK : $(BIN)

$(BIN) : $(SRC_C) $(SRC_H)
	$(CC) $(SRC_C) $(INCLUDE) $(LIBS) -o $(BIN)

.PHONY : clean

clean :
	rm $(BIN) -v
