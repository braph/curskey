#include "../curskey.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#undef  CTRL  //usr/include/sys/ttydefaults.h defines this
#define ALT   CURSKEY_MOD_META
#define SHIFT CURSKEY_MOD_SHIFT
#define CTRL  CURSKEY_MOD_CTRL
#define ARRAY_LEN(A) (sizeof(A)/sizeof(*A))

static struct {
	int keycode[8192];
	char* keyseq[8192];
	int count;
} RESULTS;

static void add_result(int keycode, char* keyseq) {
	RESULTS.keycode[RESULTS.count] = keycode;
	RESULTS.keyseq[RESULTS.count] = keyseq;
	RESULTS.count++;
}

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
	case curskey_mod_key('c', CTRL): return 1;
	case curskey_mod_key('s', CTRL): return 1;
	case curskey_mod_key('z', CTRL): return 1;
	default:                          return 0;
	}
}

static int X11_send_key(int key, int mod) {
	char def[32] = "";
	if (mod & ALT)   strcat(def, "alt+");
	if (mod & SHIFT) strcat(def, "shift+");
	if (mod & CTRL) strcat(def, "ctrl+");
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

	addstr(curskey_get_keydef(key_modded));
	napms(10);

	if (X11_send_key(key, mod)) {
		add_result(key_modded, "xdotool error");
		return;
	}

	wtimeout(stdscr, 100);
	int having_key = curskey_getch();
	if (having_key == key_modded)
		add_result(key_modded, NULL);
	else {
		wtimeout(stdscr, 0);
		while (getch() != ERR); // eat up keys

		X11_send_key(key, mod);
		keypad(stdscr, FALSE);
		char keyseq[32] = "";
		for (int c, i = 0; (c = getch()) != ERR; ++i) {
			char s[8];
			if (c <= 32 || c >= 127)
				sprintf(s, "%X", c);
			else
				sprintf(s, "%c", c);
			strcat(keyseq, s);
			strcat(keyseq, " ");
		}
		add_result(key_modded, strdup(keyseq));
		keypad(stdscr, TRUE);
	}
}

int main() {
	RESULTS.count = 0;

	initscr();
	curskey_init();
	noecho();

	const int keysyms[] = {
		KEY_UP,
		KEY_DOWN,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_PAGEUP,
		KEY_PAGEDOWN,
		KEY_HOME,
		KEY_END,
		KEY_INSERT,
		KEY_DELETE
	};

	int c;
	for (c = 0; c < ARRAY_LEN(keysyms); ++c)
		test(keysyms[c], 0);
	for (c = 'a'; c <= 'z'; ++c) test(c, 0);
	for (c = 'a'; c <= 'z'; ++c) test(c, ALT);
	//for (c = 'a'; c <= 'z'; ++c) test(c, CTRL);
	//for (c = 'a'; c <= 'z'; ++c) test(c, SHIFT);
	//for (c = 'a'; c <= 'z'; ++c) test(c, SHIFT | CTRL);
	//for (c = 'a'; c <= 'z'; ++c) test(c, SHIFT | ALT);
	for (c = 1; c <= 12; ++c)    test(KEY_F(c), 0);

	endwin();

	int tests_failed = 0;
	for (int i = 0; i < RESULTS.count; ++i) {
		tests_failed += !! RESULTS.keyseq[i];
		printf("%-20s %s%s\n",
				curskey_get_keydef(RESULTS.keycode[i]),
				(RESULTS.keyseq[i] ? "FAILED: Keysequence is " : "OK"),
				(RESULTS.keyseq[i] ? RESULTS.keyseq[i] : "")
		);
	}

	return !! tests_failed;
}
