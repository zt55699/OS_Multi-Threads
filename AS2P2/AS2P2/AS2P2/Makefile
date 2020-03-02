CC := gcc
CFLAGS := -g -Wall -Wno-deprecated-declarations -Werror

all: dataPar dataPar_single_thread

clean:
	rm -rf dataPar dataPar.dSYM dataPar_single_thread dataPar_single_thread.dSYM
	
dataPar: dataPar.c
	$(CC) $(CFLAGS) -o dataPar dataPar.c -lpthread

dataPar_single_thread: dataPar_single_thread.c
	$(CC) $(CFLAGS) -o dataPar_single_thread dataPar_single_thread.c -lpthread