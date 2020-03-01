CC := gcc
CFLAGS := -g -Wall -Wno-deprecated-declarations -Werror

all: dataPar

clean:
	rm -rf dataPar dataPar.dSYM

dataPar: dataPar.c
	$(CC) $(CFLAGS) -o dataPar dataPar.c -lpthread
