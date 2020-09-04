

curskey_test: curskey_test.c ../curskey.o
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -lcurses ../curskey.o -o curskey_test curskey_test.c
	if which valgrind; then valgrind ./curskey_test; else ./curskey_test; fi

colors: colors.c ../curskey.o
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -lcurses ../curskey.o -o colors colors.c
	if which valgrind; then valgrind ./colors; else ./colors; fi

clean:
	rm -f curskey_test
	rm -f colors
	
