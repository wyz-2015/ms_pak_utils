CC=gcc
SRC_C=common.c decompress.c compress.c main.c
SRC_H=args.h common.h compress.h decompress.h pklz.h
INCLUDE=-I./
BIN=./../bin/pklz
LIBS=

ifeq ($(OS),Windows_NT)
	LIBS+=-largp
	BIN+=.exe
endif

all : PKLZ

PKLZ : $(BIN)

$(BIN) : $(SRC_C) $(SRC_H)
	$(CC) $(SRC_C) $(INCLUDE) $(LIBS) -o $(BIN)

.PHONY : clean

clean :
	rm $(BIN) -v
