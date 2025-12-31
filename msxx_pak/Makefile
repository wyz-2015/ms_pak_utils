CC=gcc
SRC=deque.c reader.c main.c common.c #writer.c
INCLUDE=-I./
BIN=./../bin/msxx_pak
LIBS=

ifeq ($(OS),Windows_NT)
	BIN+=.exe
	LIBS+=-largp
endif

all : MSXX_PAK

MSXX_PAK : $(BIN)

$(BIN) : $(SRC)
	$(CC) $(SRC) $(INCLUDE) $(LIBS) -o $(BIN)
