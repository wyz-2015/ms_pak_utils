CC=gcc

all : PAK MSXX_PAK

PAK :
	make -C ./pak/

MSXX_PAK :
	make -C ./msxx_pak/
