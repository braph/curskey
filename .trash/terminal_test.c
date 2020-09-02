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
#define SHIFT CURSKEY_MOD_SHIFT
#define CTRL  CURSKEY_MOD_CTRL
#define ARRAY_LEN(A) ((int) (sizeof(A)/sizeof(*A)))

enum Status { E_OK, E_XDOTOOL, E_INVALID_KEYSEQ, E_TRAILING_CHARS };
const char* StatusStr[] = {"OK", "E_XDOTOOL", "E_INVALID_KEYSEQ", "E_TRAILING_CHARS"};

static struct {
	int         keycode        [8192];
	int         having_keycode [8192];
	char*       keyseq         [8192];
	char*       rest           [8192];
	enum Status status         [8192];
	int         count;
} RESULTS;

static struct {
	int keycode[8192];
	int count;
} BLACKLIST;

static void add_result(int keycode, int having, const char* keyseq, const char* rest, enum Status code) {
	RESULTS.count[RESULTS.keycode]        = keycode;
	RESULTS.count[RESULTS.having_keycode] = having;
	RESULTS.count[RESULTS.keyseq]         = strdup(keyseq);
	RESULTS.count[RESULTS.rest]           = strdup(rest);
	RESULTS.count[RESULTS.status]         = code;
	RESULTS.count++;
}

static void add_blacklist(int key_modded) {
	BLACKLIST.keycode[BLACKLIST.count++] = key_modded;
}

static int is_blacklisted(int key_modded) {
	for (int i = 0; i < BLACKLIST.count; ++i)
		if (BLACKLIST.keycode[i] == key_modded)
			return 1;
	return 0;
}

static const char* curses_keysm_to_X11_keysym_str(int key) {
	static char buf[8];
	switch (key) {
	case KEY_SPACE:     return "space";
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
	if (mod & SHIFT) strcat(def, "shift+");
	if (mod & CTRL)  strcat(def, "ctrl+");
	strcat(def, curses_keysm_to_X11_keysym_str(key));

	int pid = fork();
	switch (pid) {
	case -1: abort();
	case 0:  execlp("xdotool", "xdotool", "key", def, NULL);
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

void test(int key, int mod) {
	char keyseq[32];
	char remaining[32];
	int key_modded = curskey_mod_key(key, mod);

	if (is_blacklisted(key_modded))
		return;

	printw("[%d] %s\n", key_modded, curskey_get_keydef(key_modded));

	wtimeout(stdscr, 0); // TODO
	eat_keys(remaining, sizeof(remaining)); // TODO

	keypad(stdscr, TRUE);
	napms(100);
	wtimeout(stdscr, 1000);

	if (X11_send_key(key, mod)) {
		add_result(key_modded, 0, "", "", E_XDOTOOL);
		return;
	}

	int having_key = curskey_getch();
	wtimeout(stdscr, 0);
	eat_keys(remaining, sizeof(remaining));

	// get keydef
	keypad(stdscr, FALSE);
	X11_send_key(key, mod);
	wtimeout(stdscr, 1000);
	eat_keys(keyseq, sizeof(keyseq));

	enum Status code = E_OK;
	if (having_key != key_modded)
		code = E_INVALID_KEYSEQ;
	else if (*remaining)
		code = E_TRAILING_CHARS;
	add_result(key_modded, having_key, keyseq, remaining, code);
}

static const char* keyseq_readable(const char* seq) {
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
		
int main(int argc, char**argv) {
	RESULTS.count = 0;
	BLACKLIST.count = 0;
	const char* OUTFILE = "result.json";

	for (int opt; (opt = getopt(argc, argv, "b:o:")) != -1;)
		switch (opt) {
			case 'o': OUTFILE = optarg; break;
			case 'b': add_blacklist(curskey_parse(optarg)); break;
			default:  return printf(USAGE, argv[0]), 1;
		}

	add_blacklist(curskey_mod_key('l', ALT));
	add_blacklist(curskey_mod_key('s', ALT));
	add_blacklist(curskey_mod_key('c', CTRL));
	add_blacklist(curskey_mod_key('s', CTRL));
	add_blacklist(curskey_mod_key('z', CTRL));
	add_blacklist(KEY_F(8));
	add_blacklist(KEY_F(11));

	initscr();
	scrollok(stdscr, TRUE);
	curskey_init();
	noecho();
	char trash;
	wtimeout(stdscr, 1000); // Give the terminal 1 sec for initialization.
	eat_keys(&trash, 1);    // Some terminals send a KEY_RESIZE at the beginning.
	
	/* ==========================================================================
	 * Tests ====================================================================
	 * ========================================================================*/

	const int keys[] = {
		KEY_UP,     KEY_DOWN,     KEY_LEFT, KEY_RIGHT,
		KEY_PAGEUP, KEY_PAGEDOWN, KEY_HOME, KEY_END,   KEY_INSERT, KEY_DELETE
	};

	int c;
	for (c = 0; c < ARRAY_LEN(keys); ++c) test(keys[c], 0);
	for (c = 'a'; c <= 'z'; ++c)          test(c, 0);
	for (c = 'a'; c <= 'z'; ++c)          test(c, ALT);
	//for (c = 'a'; c <= 'z'; ++c)        test(c, CTRL);
	//for (c = 'a'; c <= 'z'; ++c)        test(c, SHIFT);
	//for (c = 'a'; c <= 'z'; ++c)        test(c, SHIFT | CTRL);
	//for (c = 'a'; c <= 'z'; ++c)        test(c, SHIFT | ALT);
	for (c = 1; c <= 12; ++c)             test(KEY_F(c), 0);

	/* ==========================================================================
	 * Output ===================================================================
	 * ========================================================================*/

	endwin();
	print_test_results(OUTFILE);
	return 0;
}
