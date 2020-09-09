#define _XOPEN_SOURCE 500
#include "../curskey.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define USAGE \
	"Usage: %s -o OUTFILE [-b BLACKLIST KEY]\n"
#undef  CTRL  //usr/include/sys/ttydefaults.h defines this
#define ALT   CURSKEY_MOD_META
#define CTRL  CURSKEY_MOD_CTRL
#define SHIFT CURSKEY_MOD_SHIFT
#define ARRAY_LEN(A) ((int) (sizeof(A)/sizeof(*A)))

enum Status { E_OK, E_BLACKLISTED, E_XDOTOOL, E_INVALID_KEYSEQ, E_TRAILING_CHARS };
const char* StatusStr[] = {"OK", "(OK)", "XDOTOOL", "INVALID_KEYSEQ", "TRAILING_CHARS"};

static struct {
	enum Status status         [8192];
	int         keycode        [8192];
	int         having_keycode [8192];
	char*       keyseq         [8192];
	char*       rest           [8192];
	int         count;
} RESULTS;

static void add_test(int key, int mod) {
	RESULTS.count[RESULTS.keycode] = curskey_mod_key(key, mod);
	RESULTS.count++;
}

static void add_blacklist(int key_modded) {
	for (int i = 0; i < RESULTS.count; ++i)
		if (RESULTS.keycode[i] == key_modded)
			RESULTS.status[i] = E_BLACKLISTED;
}

static const char* curses_keysm_to_X11_keysym_str(int key) {
	static char buf[8];
	switch (key) {
	case KEY_SPACE:     return "space";
	case KEY_TAB:       return "Tab";
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

static int X11_send_key(int key, int mod) {
	char def[32] = "";
	if (mod & ALT)   strcat(def, "alt+");
	if (mod & CTRL)  strcat(def, "ctrl+");
	if (mod & SHIFT) strcat(def, "shift+");
	strcat(def, curses_keysm_to_X11_keysym_str(key));

	fprintf(stderr, "sending:%s\n", def);//TODO

	int pid = fork();
	switch (pid) {
	case -1: abort();
	case 0:  execlp("xdotool", "xdotool", "key", "--clearmodifiers", def, NULL);
					 exit(127);
	default: waitpid(pid, &pid, 0);
					 return pid;
	}
}

static void eat_keys(char* buffer, int bufsize) {
	int c = getch();
	if (c != ERR) {
		wtimeout(stdscr, 0);
		*buffer++ = c;
		--bufsize;
		while ((c = getch()) != ERR)
			if (bufsize >= 1) {
				*buffer++ = c;
				--bufsize;
			}
	}
	*buffer = '\0';
}

static void run_get_keydefs() {
	char keyseq[32];

	keypad(stdscr, FALSE);
	napms(500); // The terminal driver needs some time after `keypad()`

	for (int i = 0; i < RESULTS.count; ++i) {
		if (RESULTS.status[i] == E_BLACKLISTED)
			continue;

		int key_modded = RESULTS.keycode[i];
		printw("[%d] %s\n", key_modded, curskey_get_keydef(key_modded));

		unsigned mod;
		int key = curskey_unmod_key(key_modded, &mod);
		X11_send_key(key, mod);
		wtimeout(stdscr, 1000);
		eat_keys(keyseq, sizeof(keyseq));
		RESULTS.keyseq[i] = strdup(keyseq);
	}
}

static void run_test() {
	char remaining[32];

	keypad(stdscr, TRUE);
	napms(500); // The terminal driver needs some time after `keypad()`

	for (int i = 0; i < RESULTS.count; ++i) {
		if (RESULTS.status[i] == E_BLACKLISTED)
			continue;

		int key_modded = RESULTS.keycode[i];
		printw("[%d] %s\n", key_modded, curskey_get_keydef(key_modded));

		unsigned mod;
		int key = curskey_unmod_key(key_modded, &mod);

		if (X11_send_key(key, mod)) {
			RESULTS.status[i] = E_XDOTOOL;
			continue;
		}

		wtimeout(stdscr, 1000);
		int having_key = curskey_getch();
		wtimeout(stdscr, 0);
		eat_keys(remaining, sizeof(remaining));

		enum Status code = E_OK;
		if (having_key != key_modded)
			code = E_INVALID_KEYSEQ;
		else if (*remaining)
			code = E_TRAILING_CHARS;
		RESULTS.status[i] = code;
		RESULTS.rest[i] = strdup(remaining);
		RESULTS.having_keycode[i] = having_key;
	}
}

static const char* keyseq_readable(const char* seq) {
	if (! seq)
		return ""; // TODO

	static char readable[32*888]; // TODO
	readable[0] = '\0';

	for (int i = 0; *seq; ++i, ++seq) {
		char s[16];
		if (*seq <= 32 || *seq >= 127)
			sprintf(s, "%X ", *seq);
		else
			sprintf(s, "%c ", *seq);
		strcat(readable, s);
	}

	readable[strlen(readable)] = '\0';
	return readable;
}

static const char* json_str(const char* s) {
	static char buffer[1024*888]; // TODO
	char* o = buffer;
	*o = '"';
	while (s && *s) {
		*++o = *s;
		++s;
	}
	*++o = '"';
	*++o = '\0';
	return buffer;
}

static void print_test_results(const char* file) {
	FILE* fh = fopen(file, "w");
	if (! fh)
		return;

	fprintf(fh, "[");
	for (int i = 0; i < RESULTS.count; ++i) {
		if (i)
			fprintf(fh, ",");
		fprintf(fh, "[");
		fprintf(fh, "%s,", json_str(curskey_get_keydef(RESULTS.keycode[i])));
		fprintf(fh, "%d,", RESULTS.keycode[i]);
		fprintf(fh, "%s,", json_str(curskey_get_keydef(RESULTS.having_keycode[i])));
		fprintf(fh, "%d,", RESULTS.having_keycode[i]);
		fprintf(fh, "%s,", json_str(keyseq_readable(RESULTS.keyseq[i])));
		fprintf(fh, "%s",  json_str(StatusStr[RESULTS.status[i]]));
		fprintf(fh, "]\n");
	}
	fprintf(fh, "]");
	fclose(fh);
}

void add_cntrl_to_blacklist(int key) {
	add_blacklist(curskey_mod_key(key, CTRL));
	add_blacklist(curskey_mod_key(key, ALT|CTRL));
	add_blacklist(curskey_mod_key(key, SHIFT|CTRL));
	add_blacklist(curskey_mod_key(key, ALT|SHIFT|CTRL));
}

int main(int argc, char**argv) {
	system("xdotool search ''  mousemove --window %1 50 50 click 1");
	freopen("/tmp/terminal_test.log", "w", stderr);
	setvbuf(stderr, NULL, _IOLBF, 0);

	const char* OUTFILE = "result.json";

	/* ==========================================================================
	 * Tests ====================================================================
	 * ========================================================================*/

	const int keys[] = {
		KEY_UP,     KEY_DOWN,     KEY_LEFT, KEY_RIGHT,
		KEY_PAGEUP, KEY_PAGEDOWN, KEY_HOME, KEY_END,   KEY_INSERT, KEY_DELETE
	};

	int c;
	// a - z
	for (c = 'a'; c <= 'z'; ++c)          add_test(c, 0);
	for (c = 'a'; c <= 'z'; ++c)          add_test(c, ALT);
	for (c = 'a'; c <= 'z'; ++c)          add_test(c, CTRL);
	for (c = 'a'; c <= 'z'; ++c)          add_test(c, SHIFT);
	for (c = 'a'; c <= 'z'; ++c)          add_test(c, ALT|CTRL);
	for (c = 'a'; c <= 'z'; ++c)          add_test(c, ALT|SHIFT);
	// F1 - F12
	for (c = 1; c <= 12; ++c)             add_test(KEY_F(c), 0);
	for (c = 1; c <= 12; ++c)             add_test(KEY_F(c), ALT);
	for (c = 1; c <= 12; ++c)             add_test(KEY_F(c), CTRL);
	for (c = 1; c <= 12; ++c)             add_test(KEY_F(c), SHIFT);
	for (c = 1; c <= 12; ++c)             add_test(KEY_F(c), SHIFT|ALT);
	for (c = 1; c <= 12; ++c)             add_test(KEY_F(c), SHIFT|CTRL);
	// Special Keys
	for (c = 0; c < ARRAY_LEN(keys); ++c) add_test(keys[c], 0);
	for (c = 0; c < ARRAY_LEN(keys); ++c) add_test(keys[c], ALT);
	for (c = 0; c < ARRAY_LEN(keys); ++c) add_test(keys[c], CTRL);
	for (c = 0; c < ARRAY_LEN(keys); ++c) add_test(keys[c], SHIFT);
	for (c = 0; c < ARRAY_LEN(keys); ++c) add_test(keys[c], CTRL|SHIFT);
	for (c = 0; c < ARRAY_LEN(keys); ++c) add_test(keys[c], ALT|CTRL);
	for (c = 0; c < ARRAY_LEN(keys); ++c) add_test(keys[c], ALT|SHIFT);
	for (c = 0; c < ARRAY_LEN(keys); ++c) add_test(keys[c], ALT|CTRL|SHIFT);

	/* ==========================================================================
	 * Commandline ==============================================================
	 * ========================================================================*/

	for (int opt; (opt = getopt(argc, argv, "b:o:")) != -1;)
		switch (opt) {
			case 'o': OUTFILE = optarg; break;
			case 'b': add_blacklist(curskey_parse(optarg)); break;
			default:  return printf(USAGE, argv[0]), 1;
		}

	add_cntrl_to_blacklist('c');
	add_cntrl_to_blacklist('s');
	add_cntrl_to_blacklist('z');

	initscr();
	scrollok(stdscr, TRUE);
	curskey_init();
	noecho();
	wtimeout(stdscr, 1000); // Give the terminal 1 sec for initialization.
  char trash;
	eat_keys(&trash, 1);    // Some terminals send a KEY_RESIZE at the beginning.

	/* ==========================================================================
	 * Run Tests ================================================================
	 * ========================================================================*/

	run_test();
	run_get_keydefs();

	/* ==========================================================================
	 * Output ===================================================================
	 * ========================================================================*/

	print_test_results(OUTFILE);
	return 0;
}
