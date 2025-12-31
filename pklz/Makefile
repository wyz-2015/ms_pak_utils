CC=gcc
SRC=common.c decompress.c compress.c main.c
INCLUDE=-I./
BIN=./../bin/pklz

all : PKLZ

PKLZ : $(BIN)

$(BIN) : $(SRC)
	$(CC) $(INCLUDE) $(SRC) -o $(BIN)
