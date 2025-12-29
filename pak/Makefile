CC=gcc
SRC=deque.c reader.c main.c writer.c common.c
INCLUDE=-I./
BIN=./../bin/ms3d_pak

all : PAK

PAK : $(BIN)

$(BIN) : $(SRC)
	$(CC) $(INCLUDE) $(SRC) -o $(BIN)
