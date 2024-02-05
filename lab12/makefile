#/*******************************************************************************
#* Name        : makefile
#* Author      : Nicholas Cali & Kyle Henderson
#* Date        : 6/24/2021
#* Description : add array using a shared library.
#* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
#******************************************************************************/
CC     = gcc
CFLAGS = -g -Wall -Werror -pedantic-errors

all: main libsum.so

# TODO: build main
main: main.c
	$(CC) $(CFLAGS) main.c -o main -ldl

# TODO: build libsum.so
libsum.so: sum.c sum.h
	$(CC) $(CFLAGS) -shared -fPIC sum.c -o libsum.so

clean:
	rm -f libsum.so main main.exe
