#include "../curskey.h"
#include <stdio.h>

int main()
{
    initscr();
	noecho();
    curskey_init();
    curskey_define_meta_keys();
    int key = getch();
    endwin();
    const char *keydef = curskey_get_keydef(key);
    printf("%s\n", keydef);
	return 0;
}

/* vim: set ts=4 sw=4 : */
