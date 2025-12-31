CC=gcc
SRC=deque.c reader.c main.c writer.c common.c
INCLUDE=-I./
BIN=./../bin/ms3d_pak
LIBS=

ifeq ($(OS),Windows_NT)
	BIN+=.exe
	LIBS+=-largp
endif

all : PAK

PAK : $(BIN)

$(BIN) : $(SRC)
	$(CC) $(SRC) $(INCLUDE) $(LIBS) -o $(BIN)
