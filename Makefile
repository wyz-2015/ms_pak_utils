all : PAK MSXX_PAK PKLZ

PAK :
	make -C ./pak/

MSXX_PAK :
	make -C ./msxx_pak/

PKLZ :
	make -C ./pklz/

clean :
	make -C ./pak/ clean
	make -C ./msxx_pak/ clean
	make -C ./pklz/ clean

.PHONY : PAK MSXX_PAK PKLZ clean
