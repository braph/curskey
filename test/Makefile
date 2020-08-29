

curskey_test: curskey_test.c
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -lcurses ../curskey.o -o curskey_test curskey_test.c
	if which valgrind; then valgrind ./curskey_test; else ./curskey_test; fi

colors: colors.c
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -lcurses ../curskey.o -o colors colors.c
	if which valgrind; then valgrind ./colors; else ./colors; fi

terminal_test: terminal_test.c
	$(CC) $(CFLAGS) -Wall -Wextra -Werror -lcurses ../curskey.o -o terminal_test terminal_test.c

clean:
	rm -f terminal_test
	rm -f curskey_test
	rm -f colors
	
#test/get_key: test/get_key.c
#	$(CC) $(CFLAGS) -Wall -Wextra -Werror -lcurses curskey.o test/get_key.c -o test/get_key
