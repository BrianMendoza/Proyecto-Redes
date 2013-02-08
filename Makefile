# 
# File:   Makefile
# Grupo 33
# Diana Vainberg 07-41618
# Brian-Marcial Mendoza 07-41206
# 
# Created on January 26, 2013, 11:19 PM
# 

CC = gcc

all: bomba

bomba : centro
	$(CC) bomba.c -o bomba

centro : centro.c
	$(CC) centro.c -o centro -lpthread
	
clean:
	rm -f bomba centro