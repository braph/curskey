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
    assert(MOD_KEY('r', CNTRL|ALT) == curskey_parse("C-M-r"));

		for (int key; (key = getch()) != -1;) {
      clear();
      switch (key) {
        case ERR:                     addstr("ERR");            break;
        case 'k':                     addstr("case 'k'");       break;
        case MOD_KEY('k', CNTRL):     addstr("case 'C-k'");     break;
        case MOD_KEY('k', ALT):       addstr("case 'M-k'");     break;
        case MOD_KEY('k', CNTRL|ALT): addstr("case 'C-A-k'");   break;
        case MOD_KEY(KEY_LEFT, CNTRL):addstr("case 'C-KEY_LEFT'"); break;
        default:                      addstr("default: ");
                                      addstr(curskey_get_keydef(key));
      }
    }

		curskey_destroy();
	}
}
