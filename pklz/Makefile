CC=gcc
SRC=common.c decompress.c compress.c main.c
INCLUDE=-I./
BIN=./../bin/pklz
LIBS=

ifeq ($(OS),Windows_NT)
	LIBS+=-largp
	BIN+=.exe
endif

all : PKLZ

PKLZ : $(BIN)

$(BIN) : $(SRC)
	$(CC) $(SRC) $(INCLUDE) $(LIBS) -o $(BIN)
