CC = gcc

all: bomba

bomba : centro
	$(CC) bomba.c -o bomba

centro : centro.c
	$(CC) centro.c -o centro -lpthread
	
clean:
	rm -f bomba centro