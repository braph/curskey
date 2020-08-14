all: build

build:
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -c curskey.c -o curskey.o

test: build
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -lcurses curskey.o curskey_test.c -o curskey_test
	if which valgrind; then valgrind ./curskey_test; else ./curskey_test; fi

example: build
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -lcurses curskey.o curskey_example.c -o curskey_example

doc: curskey.h
	doxygen documentation.doxy

clean:
	rm -f  curskey.o
	rm -f  curskey_test
	rm -f  curskey_example
	rm -rf doc
