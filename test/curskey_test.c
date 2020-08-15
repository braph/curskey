#include <stdlib.h>
#include <ncurses.h>
#include <term.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../curskey.h"

int count = 0;					// Test count
int having_int; 				// Test result, int
const char* having_str;			// Test result, str
const char* test_info = NULL;	// Additional info string

#define print_test(FMT, ARGS...) do { \
	if (test_info) \
		printf("[%3d] Testing " FMT " [%s] ... ", ++count, ARGS, test_info); \
	else \
		printf("[%3d] Testing " FMT " ... ", ++count, ARGS); \
} while (0)

// Check for `EXPR != ERR`
#define test_no_err(EXPR) do { \
	print_test("ERR != %s", #EXPR); \
	if (ERR == (EXPR)) { \
		printf("Failed.\n\tExpected:\tNon-ERR\n\tHaving:  \t%d\n", \
			having_int); \
		assert(!#EXPR); \
	} else printf("OK.\n"); \
} while(0)

// Check for `EXPR == EXPECT_INT`
#define test_int(EXPECT_INT, EXPR) do { \
	print_test("%s [%d] == %s", #EXPECT_INT, EXPECT_INT, #EXPR); \
	if ((having_int = (EXPR)) != EXPECT_INT) { \
		printf("Failed.\n\tExpected:\t%d\n\tHaving:  \t%d\n", \
			EXPECT_INT, having_int); \
		assert(!#EXPR); \
	} else printf("OK.\n"); \
} while(0)

// Check for `EXPR == EXPECT_STR`
#define test_str(EXPECT_STR, EXPR) do { \
	print_test("%s == %s", #EXPECT_STR, #EXPR); \
	if (!streq((having_str = (EXPR)), EXPECT_STR)) { \
		printf("Failed.\n\tExpected:\t%s\n\tHaving:  \t%s\n", \
			(EXPECT_STR ? EXPECT_STR : "(NULL)"), having_str); \
		assert(!#EXPR); \
	} else { \
		printf("OK.\n"); \
	} \
} while(0)

static int streq(const char *a, const char *b) {
	return ((!a || !b) ? a == b : !strcmp(a, b));
}

#define MOD_META	CURSKEY_MOD_META
#define MOD_CNTRL	CURSKEY_MOD_CNTRL

void do_tests() {
	char buf[128];

	// ========================================================================
	// curskey_normalize() ====================================================
	// ========================================================================
#if 0
#define test test_str
	test (NULL,   curskey_normalize(NULL));
	test ("F1",   curskey_normalize("KEY_F1"));
	test ("F1",   curskey_normalize("KEY_F(1)"));
	test ("f12",  curskey_normalize("KEY_f(12)"));
	test ("HOME", curskey_normalize("HOME"));
	test ("HOME", curskey_normalize("KEY_HOME"));
#undef test
#endif

#define test test_int

	// ========================================================================
	// curskey_keycode() ======================================================
	// ========================================================================

#if 0
	test (127,           curskey_keycode("DEL"));      // aliased
	test (' ',           curskey_keycode("SPACE"));    // aliased
	test ('\t',          curskey_keycode("TAB"));      // aliased
	test ('\n',          curskey_keycode("RETURN"));   // aliased
	test (KEY_ESCAPE,    curskey_keycode("ESCAPE"));   // aliased
	test (KEY_HOME,      curskey_keycode("HOME"));
	test (KEY_PAGEUP,    curskey_keycode("PAGEUP"));   // aliased
	test (KEY_PAGEDOWN,  curskey_keycode("PAGEDOWN")); // aliased
	test (KEY_INSERT,    curskey_keycode("INSERT"));   // aliased
	test (KEY_DELETE,    curskey_keycode("DELETE"));   // aliased
	test (KEY_UP,        curskey_keycode("UP"));
	test (KEY_DOWN,      curskey_keycode("DOWN"));
	test (KEY_LEFT,      curskey_keycode("LEFT"));
	test (KEY_RIGHT,     curskey_keycode("RIGHT"));
	test (KEY_F(1),      curskey_keycode("F1"));
	test (KEY_F(12),     curskey_keycode("F12"));
	test (KEY_F(63),     curskey_keycode("F63"));

	// different function key format
	test (KEY_F(1),      curskey_keycode("KEY_F1"));
	test (KEY_F(1),      curskey_keycode("KEY_F(1)"));
	test (KEY_F(1),      curskey_keycode("key_f(1)"));
	test (KEY_F(1),      curskey_keycode("F(1)"));
	test (KEY_F(1),      curskey_keycode("f(1)"));

	// different case
	test (KEY_HOME,      curskey_keycode("KEY_HOME"));
	test (KEY_HOME,      curskey_keycode("KeY_HOME"));
	test (KEY_HOME,      curskey_keycode("key_home"));
	test (KEY_HOME,      curskey_keycode("home"));

	// invalid keys
	//test (ERR,           curskey_keycode(NULL));
	test (ERR,           curskey_keycode(""));
	test (ERR,           curskey_keycode("a"));
	test (ERR,           curskey_keycode("foo"));
	test (ERR,           curskey_keycode("F64"));
#endif

	// ========================================================================
	// curskey_cntrl_key() ====================================================
	// ========================================================================
	test (0,             curskey_cntrl_key(KEY_SPACE));
	test (1,             curskey_cntrl_key('a'));
	test (1,             curskey_cntrl_key('A'));
	test (26,            curskey_cntrl_key('z'));
	test (26,            curskey_cntrl_key('Z'));
	test (ERR,           curskey_cntrl_key(-1));
	test (ERR,           curskey_cntrl_key(0));
	test (ERR,           curskey_cntrl_key(33));
	test (ERR,           curskey_cntrl_key(128));
	test (ERR,           curskey_cntrl_key(KEY_TAB));
	test (ERR,           curskey_cntrl_key(KEY_RETURN));
	test (ERR,           curskey_cntrl_key(KEY_DEL));
	test (ERR,           curskey_cntrl_key(KEY_ESCAPE));

	// ========================================================================
	// curskey_meta_key() =====================================================
	// ========================================================================
	test (ERR,          curskey_meta_key(-1));
	test (ERR,          curskey_meta_key(128));
	test_no_err (       curskey_meta_key('a'));
	test_no_err (       curskey_meta_key('A'));
	test_no_err (       curskey_meta_key(KEY_SPACE));
	test_no_err (       curskey_meta_key(KEY_TAB));
	test_no_err (       curskey_meta_key(KEY_RETURN));
	test_no_err (       curskey_meta_key(KEY_DEL));
	test_no_err (       curskey_meta_key(KEY_ESCAPE));

	// ========================================================================
	// curskey_mod_key() - control + meta =====================================
	// ========================================================================
	test_no_err (curskey_mod_key('a',       MOD_META|MOD_CNTRL));
	test_no_err (curskey_mod_key('A',       MOD_META|MOD_CNTRL));
	test_no_err (curskey_mod_key('z',       MOD_META|MOD_CNTRL));
	test_no_err (curskey_mod_key('Z',       MOD_META|MOD_CNTRL));
	test (ERR, curskey_mod_key(-1,          MOD_META|MOD_CNTRL));
	test (ERR, curskey_mod_key(0,           MOD_META|MOD_CNTRL));
	test (ERR, curskey_mod_key(33,          MOD_META|MOD_CNTRL));
	test (ERR, curskey_mod_key(128,         MOD_META|MOD_CNTRL));
	test_no_err (curskey_mod_key(KEY_SPACE, MOD_META|MOD_CNTRL));
	test (ERR, curskey_mod_key(KEY_TAB,     MOD_META|MOD_CNTRL));
	test (ERR, curskey_mod_key(KEY_RETURN,  MOD_META|MOD_CNTRL));
	test (ERR, curskey_mod_key(KEY_DEL,     MOD_META|MOD_CNTRL));
	test (ERR, curskey_mod_key(KEY_ESCAPE,  MOD_META|MOD_CNTRL));

	// ========================================================================
	// curskey_parse() ========================================================
	// ========================================================================

	// invalid
	test (ERR, curskey_parse(""));

	// simple character
	test ('a',  curskey_parse("a"));
	test ('A',  curskey_parse("A"));
	buf[1] = '\0';
	test_info = buf;
	for (buf[0] = 32; buf[0] < 127; ++buf[0]) {
		test(buf[0], curskey_parse(buf));
	}
	test_info = NULL;

	// control character
	test (1,  curskey_parse("^a"));
	test (1,  curskey_parse("^A"));
	test (1,  curskey_parse("C-a"));
	test (1,  curskey_parse("c-a"));
	test (1,  curskey_parse("C-A"));
	test (1,  curskey_parse("c-A"));
	test (26, curskey_parse("c-Z"));
	test (26, curskey_parse("c-z"));
	test (27, curskey_parse("c-["));
	test (28, curskey_parse("c-\\"));
	test (29, curskey_parse("c-]"));
	test (30, curskey_parse("c-^"));
	test (31, curskey_parse("c-_"));
	
	// invalid control characters
	buf[0] = 'C';
	buf[1] = '-';
	buf[3] = '\0';
	test_info = buf;
	for (buf[2] = 33; buf[2] < 'A'; ++buf[2])
		test(ERR, curskey_parse(buf));
	for (buf[2] = '{'; buf[2] < 127; ++buf[2])
		test(ERR, curskey_parse(buf));
	test_info = NULL;

	// control special
	test (0, curskey_parse("C-SPACE"));
	// invalid keys (symbolic with cntrl is not supported)
	test (ERR, curskey_parse("C-TAB"));
	test (ERR, curskey_parse("C-RETURN"));
	test (ERR, curskey_parse("C-DEL"));
	test (ERR, curskey_parse("C-ESCAPE"));
	test (ERR, curskey_parse("C-INSERT"));
	test (ERR, curskey_parse("C-LEFT"));
	test (ERR, curskey_parse("C-HOME"));

	// meta
	test (curskey_meta_key('a'),  curskey_parse("M-a"));
	test (curskey_meta_key('a'),  curskey_parse("m-a"));
	test (curskey_meta_key('A'),  curskey_parse("M-A"));
	test (curskey_meta_key('A'),  curskey_parse("m-A"));

	// meta special ("fake" special, since these are single characters)
	test (curskey_meta_key(KEY_SPACE),  curskey_parse("M-SPACE"));
	test (curskey_meta_key(KEY_TAB),    curskey_parse("M-TAB"));
	test (curskey_meta_key(KEY_RETURN), curskey_parse("M-RETURN"));
	test (curskey_meta_key(KEY_DEL),    curskey_parse("M-DEL"));
	test (curskey_meta_key(KEY_ESCAPE), curskey_parse("M-ESCAPE"));

	// invalid keys (symbolic with meta is not supported)
	test (ERR, curskey_parse("M-INSERT"));
	test (ERR, curskey_parse("M-LEFT"));
	test (ERR, curskey_parse("M-HOME"));

	// meta + control
	test (curskey_mod_key(KEY_SPACE, MOD_META|MOD_CNTRL), curskey_parse("C-M-SPACE"));
	test (ERR,          curskey_parse("C-M-TAB"));
	test (ERR,          curskey_parse("C-M-RETURN"));
	test (ERR,          curskey_parse("C-M-DEL"));
	test (ERR,          curskey_parse("C-M-ESCAPE"));

	// ncurses keys
	test (' ',	        curskey_parse("SPACE"));
	test ('\t',	        curskey_parse("TAB"));
	test ('\n',	        curskey_parse("RETURN"));
	test (KEY_ESCAPE,	curskey_parse("ESCAPE"));
	test (KEY_DEL,	    curskey_parse("DEL"));
	test (KEY_UP,	    curskey_parse("UP"));
	test (KEY_DOWN,	    curskey_parse("DOWN"));
	test (KEY_LEFT,	    curskey_parse("LEFT"));
	test (KEY_RIGHT,	curskey_parse("RIGHT"));
	test (KEY_PPAGE,	curskey_parse("PAGEUP"));
	test (KEY_NPAGE,	curskey_parse("PAGEDOWN"));
	test (KEY_END,	    curskey_parse("END"));
	test (KEY_HOME,	    curskey_parse("HOME"));
	test (KEY_DC,	    curskey_parse("DELETE"));
	test (KEY_IC,	    curskey_parse("INSERT"));
	test (KEY_F(1),     curskey_parse("F1"));
	test (KEY_F(63),    curskey_parse("F63"));
	test (ERR,          curskey_parse("F64"));

#undef test
#define test test_str
	// curskey_keyname
#if 0 // TODO
	test("SPACE",       curskey_keyname(' '));
	test("TAB",         curskey_keyname('\t'));
	test("DEL",         curskey_keyname(127));
	test("ESCAPE",      curskey_keyname(27));
#endif

	// ========================================================================
	// curskey_get_keydef() ===================================================
	// ========================================================================

	// single characters
	test("a",           curskey_get_keydef('a'));
	test("A",           curskey_get_keydef('A'));

	// ncurses keynames
	test("HOME",        curskey_get_keydef(KEY_HOME));
	test("F1",          curskey_get_keydef(KEY_F(1)));
	test("F10",         curskey_get_keydef(KEY_F(10)));
	test("F33",         curskey_get_keydef(KEY_F(33)));
	test("F63",         curskey_get_keydef(KEY_F(63)));

	// control characters
	test("C-A",         curskey_get_keydef(1));
	test("C-Z",         curskey_get_keydef(26));
	test("ESCAPE",      curskey_get_keydef(27)); // special case
	test("C-\\",        curskey_get_keydef(28));
	test("C-]",         curskey_get_keydef(29));
	test("C-^",         curskey_get_keydef(30));
	test("C-_",         curskey_get_keydef(31));

	// meta characters
	test("M-a",         curskey_get_keydef(curskey_meta_key('a')));
	test("M-A",         curskey_get_keydef(curskey_meta_key('A')));
	test("M-z",         curskey_get_keydef(curskey_meta_key('z')));
	test("M-Z",         curskey_get_keydef(curskey_meta_key('Z')));
	test("M-\\",        curskey_get_keydef(curskey_meta_key('\\')));
	test("M-]",         curskey_get_keydef(curskey_meta_key(']')));
	test("M-^",         curskey_get_keydef(curskey_meta_key('^')));
	test("M-_",         curskey_get_keydef(curskey_meta_key('_')));

	// meta + control characters
	test("C-M-A",       curskey_get_keydef(curskey_mod_key('a', MOD_CNTRL|MOD_META)));
	test("C-M-A",       curskey_get_keydef(curskey_mod_key('A', MOD_CNTRL|MOD_META)));
	test("C-M-Z",       curskey_get_keydef(curskey_mod_key('z', MOD_CNTRL|MOD_META)));
	test("C-M-Z",       curskey_get_keydef(curskey_mod_key('Z', MOD_CNTRL|MOD_META)));
	test("C-M-\\",      curskey_get_keydef(curskey_mod_key('\\',MOD_CNTRL|MOD_META)));
	test("C-M-]",       curskey_get_keydef(curskey_mod_key(']', MOD_CNTRL|MOD_META)));
	test("C-M-^",       curskey_get_keydef(curskey_mod_key('^', MOD_CNTRL|MOD_META)));
	test("C-M-_",       curskey_get_keydef(curskey_mod_key('_', MOD_CNTRL|MOD_META)));

	// special keynames
	test("SPACE",       curskey_get_keydef(KEY_SPACE));
	test("DEL",         curskey_get_keydef(KEY_DEL));
	test("TAB",         curskey_get_keydef(KEY_TAB));
	test("RETURN",      curskey_get_keydef(KEY_RETURN));
	test("ESCAPE",      curskey_get_keydef(KEY_ESCAPE));

	test("M-SPACE",     curskey_get_keydef(curskey_meta_key(KEY_SPACE)));
	test("M-DEL",       curskey_get_keydef(curskey_meta_key(KEY_DEL)));
	test("M-TAB",       curskey_get_keydef(curskey_meta_key(KEY_TAB)));
	test("M-RETURN",    curskey_get_keydef(curskey_meta_key(KEY_RETURN)));
	test("M-ESCAPE",    curskey_get_keydef(curskey_meta_key(KEY_ESCAPE)));

	test("C-SPACE",     curskey_get_keydef(curskey_cntrl_key(KEY_SPACE)));
	test(NULL,	        curskey_get_keydef(curskey_cntrl_key(KEY_DEL)));
	test(NULL,          curskey_get_keydef(curskey_cntrl_key(KEY_TAB)));
	test(NULL,          curskey_get_keydef(curskey_cntrl_key(KEY_RETURN)));
	test(NULL,          curskey_get_keydef(curskey_cntrl_key(KEY_ESCAPE)));

	test(NULL,          curskey_get_keydef(KEY_MAX * 2));
}

void print_keys() {
	int i;
	const char *keydef;

	for (i = 0; i < CURSKEY_KEY_MAX; ++i)
	{
		if ((keydef = curskey_get_keydef(i))) {
			printf("%03d = %-10s = %-10s\n", i, keydef, keyname(i));
		}
	}

	printf("\n");
}

int usage(const char *p) {
	printf("Usage: %s\n\t-d\tdump keycodes\n\t-i\tinteractive\n", p);
	return 1;
}

int main(int argc, char *argv[])
{
	int opt_dump = 0;
	int opt_interactive = 0;

	for (int i = 1; i < argc; ++i)
		if (!strcmp(argv[i], "-i"))
			opt_interactive = 1;
		else if (!strcmp(argv[i], "-d"))
			opt_dump = 1;
		else
			return usage(argv[0]);

	for (int TRUE_FALSE = 0; TRUE_FALSE <= 1; ++TRUE_FALSE) {
		initscr();
		curskey_init();
        curskey_define_meta_keys(128);
		meta(stdscr, TRUE_FALSE);
		endwin();
		if (opt_dump)
			print_keys();
		do_tests();
		printf("\tcurskey has been tested with `meta(stdscr, %s)`\n\n",
				(TRUE_FALSE ? "ON" : "OFF"));
		curskey_destroy();
	}

	initscr();
	curskey_init();
	//initscr();
	noecho();
	//cbreak();
	//nonl();

	if (opt_interactive) {
		addstr("Interactive test\n");

		for (int ch = getch() ; ch != -1 ; ch = getch()) {
			char buf[64];
			const char *keydef = curskey_get_keydef(ch);
			const char *nc_keyname = keyname(ch);
			sprintf(buf, "%d curskey: %s ncurses: %s\n", ch,
					(keydef ? keydef : "???"),
					(nc_keyname ? nc_keyname : "???"));

			addstr(buf);

			if (ch == 13)
				addch('\n');
			else if (ch == 'q' || ch == 'Q')
				break;

			refresh();
		}
	}

	curskey_destroy();
	return 0;
}

/* vim: set ts=4 sw=4 : */
