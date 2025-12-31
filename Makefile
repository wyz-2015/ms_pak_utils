CC=gcc

all : PAK MSXX_PAK PKLZ

PAK :
	make -C ./pak/

MSXX_PAK :
	make -C ./msxx_pak/

PKLZ :
	make -C ./pklz/
