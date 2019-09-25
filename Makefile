all: build

build:
	$(CC) -Wall -g -lcurses -c curskey.c -o curskey.o

test:
	$(CC) -Wall -g -lcurses curskey.o curskey_test.c -o curskey_test
	$(CC) -Wall -g -lcurses curskey.o curskey_example.c -o curskey_example
	if which valgrind; then valgrind ./curskey_test; else ./curskey_test; fi

clean:
	rm -f curskey.o
	rm -f curskey_test
	rm -f curskey_example
