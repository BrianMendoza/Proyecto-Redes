CC = gcc

all: bomba

bomba : centro
	$(CC) bomba.c -o bomba

centro : centro.c
	$(CC) centro.c -o centro
	
clean:
	rm -f bomba centro



# include project implementation makefile
include nbproject/Makefile-impl.mk

# include project make variables
include nbproject/Makefile-variables.mk
