#include "../curskey.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define ALT   CURSKEY_MOD_META
#define SHIFT CURSKEY_MOD_SHIFT
#define CNTRL CURSKEY_MOD_CNTRL

static int TESTS_FAILED = 0;
static char* OUTPUT;

static const char* curses_keysm_to_X11_keysym_str(int key) {
	static char buf[16];
	switch (key) {
	case KEY_ESCAPE:    return "Escape";
	case KEY_BACKSPACE: return "BackSpace";
	case KEY_ENTER:     return "KP_Enter";
	case KEY_NPAGE: 		return "Next";
	case KEY_PPAGE:     return "Prior";
	case KEY_LEFT:      return "Left";
	case KEY_RIGHT:     return "Right";
	case KEY_UP:        return "Up";
	case KEY_DOWN:      return "Down";
	case KEY_HOME:      return "Home";
	case KEY_END:       return "End";
	case KEY_IC:        return "Insert";
	case KEY_DC:        return "Delete";
//case KEY_RETURN:    return "Return"; TODO
	default:
		if (key >= KEY_F(1) && key <= KEY_F(63))
			sprintf(buf, "F%d", key - KEY_F(0));
		else
			sprintf(buf, "%c", key);

		return buf;
	}
}

static int is_blacklisted(int key_modded) {
	switch (key_modded) {
	case curskey_mod_key('l', ALT):   return 1;
	case curskey_mod_key('s', ALT):   return 1;
	case curskey_mod_key('c', CNTRL): return 1;
	case curskey_mod_key('s', CNTRL): return 1;
	case curskey_mod_key('z', CNTRL): return 1;
	default:                          return 0;
	}
}

static int X11_send_key(int key, int mod) {
	char def[32] = "";
	if (mod & ALT)   strcat(def, "alt+");
	if (mod & SHIFT) strcat(def, "shift+");
	if (mod & CNTRL) strcat(def, "ctrl+");
	strcat(def, curses_keysm_to_X11_keysym_str(key));

	int pid = fork();
	switch (pid) {
	case -1: abort();
	case 0:  execlp("xdotool", "xdotool", "key", def);
					 exit(127);
	default: waitpid(pid, &pid, 0);
					 return pid;
	}
}

void test(int key, int mod) {
	int key_modded = curskey_mod_key(key, mod);

	if (is_blacklisted(key_modded))
		return;

	strcat(OUTPUT, "Test: ");
	strcat(OUTPUT, curskey_get_keydef(key_modded));
	strcat(OUTPUT, " ... ");

	addstr(curskey_get_keydef(key_modded));
	napms(100);

	if (X11_send_key(key, mod)) {
		TESTS_FAILED++;
		strcat(OUTPUT, "FAILED: xdotool error\n");
		return;
	}

	wtimeout(stdscr, 100);
	int having_key = curskey_getch();
	if (having_key == key_modded)
		strcat(OUTPUT, "OK\n");
	else {
		TESTS_FAILED++;
		strcat(OUTPUT, "FAILED: Key sequence is: ");

		wtimeout(stdscr, 0);
		while (getch() != ERR); // eat up keys

		X11_send_key(key, mod);
		keypad(stdscr, FALSE);
		for (int c; (c = getch()) != ERR;) {
			char s[8];
			if (c <= 32 || c >= 127)
				sprintf(s, "%X", c);
			else
				sprintf(s, "%c", c);
			strcat(OUTPUT, s);
			strcat(OUTPUT, " ");
		}
		strcat(OUTPUT, "\n");
		keypad(stdscr, TRUE);
	}
}

int main() {
	initscr();
	curskey_init();
	noecho();

	OUTPUT = malloc(1024*1024*5);
	*OUTPUT = '\0';

	int c;
	test(KEY_UP, 0);
	test(KEY_DOWN, 0);
	test(KEY_LEFT, 0);
	test(KEY_RIGHT, 0);
	test(KEY_HOME, 0);
	//for (c = 'a'; c <= 'z'; ++c) test(c, 0);
	//for (c = 'a'; c <= 'z'; ++c) test(c, ALT);
	//for (c = 'a'; c <= 'z'; ++c) test(c, CNTRL);
	//for (c = 'a'; c <= 'z'; ++c) test(c, SHIFT);
	//for (c = 'a'; c <= 'z'; ++c) test(c, SHIFT | CNTRL);
	//for (c = 'a'; c <= 'z'; ++c) test(c, SHIFT | ALT);

	endwin();
	puts(OUTPUT);
	return !! TESTS_FAILED;
}
