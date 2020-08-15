all: build

build:
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -c curskey.c -o curskey.o

test: build
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -lcurses curskey.o -o test.out test/curskey_test.c
	if which valgrind; then valgrind ./test.out; else ./test.out; fi
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -lcurses curskey.o -o test.out test/colors.c
	if which valgrind; then valgrind ./test.out; else ./test.out; fi
	rm -f test.out

example: build
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -lcurses curskey.o curskey_example.c -o curskey_example

doc: curskey.h
	doxygen documentation.doxy

clean:
	rm -f  curskey.o
	rm -f  curskey_test
	rm -f  curskey_example
	rm -rf doc
