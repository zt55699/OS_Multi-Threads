CC := gcc
CFLAGS := -g -Wall -Wno-deprecated-declarations -Werror

all: dataPar test_single_thread test1 test2

clean:
	rm -rf dataPar dataPar.dSYM test_single_thread test_single_thread.dSYM test1 test1.dSYM test2 test2.dSYM
	
dataPar: dataPar.c
	$(CC) $(CFLAGS) -o dataPar dataPar.c -lpthread

test_single_thread: test_single_thread.c
	$(CC) $(CFLAGS) -o test_single_thread test_single_thread.c -lpthread
	
test1: test1.c
	$(CC) $(CFLAGS) -o test1 test1.c -lpthread
	
test2: test2.c
	$(CC) $(CFLAGS) -o test2 test2.c -lpthread