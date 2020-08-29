#include "curskey.h"
#include <assert.h>

#define CNTRL   CURSKEY_MOD_CNTRL
#define ALT     CURSKEY_MOD_META
#define SHIFT   CURSKEY_MOD_SHIFT
#define MOD_KEY curskey_mod_key

int main() {
	initscr(); // Has to be called!
	noecho();

	if (curskey_init() == OK) {
    endwin();

    printf("%d\n", MOD_KEY('k', 0));
    printf("%d\n", MOD_KEY('k', CNTRL|ALT));
    printf("%d\n", MOD_KEY('k', ALT));

		curskey_destroy();
	}
}
