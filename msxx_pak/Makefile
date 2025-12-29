CC=gcc
SRC=deque.c reader.c main.c common.c #writer.c
INCLUDE=-I./
BIN=./../bin/msxx_pak

all : MSXX_PAK

MSXX_PAK : $(BIN)

$(BIN) : $(SRC)
	$(CC) $(INCLUDE) $(SRC) -o $(BIN)
