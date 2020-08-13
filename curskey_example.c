#include "curskey.h"
#include <assert.h>

int main() {
	initscr(); // Has to be called!
	noecho();
	if (curskey_init() == OK) {
		//int key_alt_r = curskey_meta_key('r');
		//int key_ctrl_r = curskey_cntrl_key('r');
		//int key_ctrl_alt_r = curskey_mod_key('r', CURSKEY_MOD_ALT|CURSKEY_MOD_CNTRL);

		int key0 = curskey_parse("C-M-r");
		int key1 = curskey_mod_key('r', CURSKEY_MOD_ALT|CURSKEY_MOD_CNTRL);
		assert(key0 == key1);

		for (int key; (key = getch()) != -1;)
			if (key == key0)
				addstr("Control-Meta-R pressed");
			else
				addstr(curskey_get_keydef(key));
		curskey_destroy();
	}
}
