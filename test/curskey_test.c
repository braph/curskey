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

#define META	CURSKEY_MOD_META
#define CNTRL	CURSKEY_MOD_CNTRL
#define SHIFT   CURSKEY_MOD_SHIFT

void do_tests() {
	char buf[128];

#define test test_int

	// ========================================================================
	// curskey_mod_key(key, CNTRL) ========================================
	// ========================================================================

	test (0,             curskey_mod_key(KEY_SPACE,     CNTRL));
	test (1,             curskey_mod_key('a',           CNTRL));
	test (1,             curskey_mod_key('A',           CNTRL));
	test (26,            curskey_mod_key('z',           CNTRL));
	test (26,            curskey_mod_key('Z',           CNTRL));
	test (ERR,           curskey_mod_key(-1,            CNTRL));
	test (ERR,           curskey_mod_key(0,             CNTRL));
	test (ERR,           curskey_mod_key(33,            CNTRL));
	test (ERR,           curskey_mod_key(128,           CNTRL));
	test (ERR,           curskey_mod_key(KEY_TAB,       CNTRL));
	test (ERR,           curskey_mod_key(KEY_RETURN,    CNTRL));
	test (ERR,           curskey_mod_key(KEY_ESCAPE,    CNTRL));
//	test (ERR,           curskey_mod_key(KEY_BACKSPACE, CNTRL)); TODO
    test_no_err (        curskey_mod_key(KEY_HOME,      CNTRL));
    test_no_err (        curskey_mod_key(KEY_END,       CNTRL));
    test_no_err (        curskey_mod_key(KEY_F(1),      CNTRL));
    test_no_err (        curskey_mod_key(KEY_F(12),     CNTRL));

	// ========================================================================
	// curskey_mod_key(KEY, META) =========================================
	// ========================================================================
	//test (ERR,           curskey_mod_key(0,          META)); TODO
	test (ERR,           curskey_mod_key(128,           META));
	test_no_err (        curskey_mod_key('a',           META));
	test_no_err (        curskey_mod_key('A',           META));
	test_no_err (        curskey_mod_key(KEY_SPACE,     META));
	test_no_err (        curskey_mod_key(KEY_TAB,       META));
	test_no_err (        curskey_mod_key(KEY_RETURN,    META));
	test_no_err (        curskey_mod_key(KEY_BACKSPACE, META));
	test_no_err (        curskey_mod_key(KEY_ESCAPE,    META));
    // TODO: symbolic keys

	// ========================================================================
	// curskey_mod_key() - control + meta =====================================
	// ========================================================================
	test_no_err (curskey_mod_key('a',            META|CNTRL));
	test_no_err (curskey_mod_key('A',            META|CNTRL));
	test_no_err (curskey_mod_key('z',            META|CNTRL));
	test_no_err (curskey_mod_key('Z',            META|CNTRL));
	test (ERR,   curskey_mod_key(-1,             META|CNTRL));
	test (ERR,   curskey_mod_key(0,              META|CNTRL));
	test (ERR,   curskey_mod_key(33,             META|CNTRL));
	test (ERR,   curskey_mod_key(128,            META|CNTRL));
	test_no_err (curskey_mod_key(KEY_SPACE,      META|CNTRL));
	test (ERR,   curskey_mod_key(KEY_TAB,        META|CNTRL));
	test (ERR,   curskey_mod_key(KEY_RETURN,     META|CNTRL));
//  test (ERR,   curskey_mod_key(KEY_BACKSPACE,  META|CNTRL)); TODO
	test (ERR,   curskey_mod_key(KEY_ESCAPE,     META|CNTRL));

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
	//test (ERR, curskey_parse("C-INSERT")); TODO
	//test (ERR, curskey_parse("C-LEFT")); TODO
	//test (ERR, curskey_parse("C-HOME")); TODO

	// meta
	test (curskey_mod_key('a', META),  curskey_parse("M-a"));
	test (curskey_mod_key('a', META),  curskey_parse("m-a"));
	test (curskey_mod_key('A', META),  curskey_parse("M-A"));
	test (curskey_mod_key('A', META),  curskey_parse("m-A"));

	// meta special ("fake" special, since these are single characters)
	test (curskey_mod_key(KEY_SPACE,     META),  curskey_parse("M-SPACE"));
	test (curskey_mod_key(KEY_TAB,       META),  curskey_parse("M-TAB"));
	test (curskey_mod_key(KEY_RETURN,    META),  curskey_parse("M-RETURN"));
	test (curskey_mod_key(KEY_BACKSPACE, META),  curskey_parse("M-BACKSPACE"));
	test (curskey_mod_key(KEY_ESCAPE,    META),  curskey_parse("M-ESCAPE"));

	// invalid keys (symbolic with meta is not supported)
	//test (ERR, curskey_parse("M-INSERT")); TODO
	//test (ERR, curskey_parse("M-LEFT")); TODO
	//test (ERR, curskey_parse("M-HOME")); TODO

	// meta + control
	test (curskey_mod_key(KEY_SPACE, META|CNTRL), curskey_parse("C-M-SPACE"));
	test (ERR,          curskey_parse("C-M-TAB"));
	test (ERR,          curskey_parse("C-M-RETURN"));
	test (ERR,          curskey_parse("C-M-DEL")); // TODO...
	test (ERR,          curskey_parse("C-M-ESCAPE"));

	// ncurses keys
	test (' ',	           curskey_parse("SPACE"));
	test ('\t',	           curskey_parse("TAB"));
	test ('\n',	           curskey_parse("RETURN"));
	test (KEY_ESCAPE,	   curskey_parse("ESCAPE"));
	test (KEY_BACKSPACE,   curskey_parse("BACKSPACE"));
	test (KEY_UP,	       curskey_parse("UP"));
	test (KEY_DOWN,	       curskey_parse("DOWN"));
	test (KEY_LEFT,	       curskey_parse("LEFT"));
	test (KEY_RIGHT,	   curskey_parse("RIGHT"));
	test (KEY_PPAGE,	   curskey_parse("PAGEUP"));
	test (KEY_NPAGE,	   curskey_parse("PAGEDOWN"));
	test (KEY_END,	       curskey_parse("END"));
	test (KEY_HOME,	       curskey_parse("HOME"));
	test (KEY_DC,	       curskey_parse("DELETE"));
	test (KEY_IC,	       curskey_parse("INSERT"));
	test (KEY_F(1),        curskey_parse("F1"));
	test (KEY_F(63),       curskey_parse("F63"));
	test (ERR,             curskey_parse("F64"));

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

	// control characters [0 - 31]
    test("C-SPACE",     curskey_get_keydef(0));
	test("C-A",         curskey_get_keydef(1));
	test("C-Z",         curskey_get_keydef(26));
	test("ESCAPE",      curskey_get_keydef(27)); // special case
	test("C-\\",        curskey_get_keydef(28));
	test("C-]",         curskey_get_keydef(29));
	test("C-^",         curskey_get_keydef(30));
	test("C-_",         curskey_get_keydef(31));

	// meta characters
	test("M-a",         curskey_get_keydef(curskey_mod_key('a',  META)));
	test("M-A",         curskey_get_keydef(curskey_mod_key('A',  META)));
	test("M-z",         curskey_get_keydef(curskey_mod_key('z',  META)));
	test("M-Z",         curskey_get_keydef(curskey_mod_key('Z',  META)));
	test("M-\\",        curskey_get_keydef(curskey_mod_key('\\', META)));
	test("M-]",         curskey_get_keydef(curskey_mod_key(']',  META)));
	test("M-^",         curskey_get_keydef(curskey_mod_key('^',  META)));
	test("M-_",         curskey_get_keydef(curskey_mod_key('_',  META)));

	// meta + control characters
	test("C-M-A",       curskey_get_keydef(curskey_mod_key('a', CNTRL|META)));
	test("C-M-A",       curskey_get_keydef(curskey_mod_key('A', CNTRL|META)));
	test("C-M-Z",       curskey_get_keydef(curskey_mod_key('z', CNTRL|META)));
	test("C-M-Z",       curskey_get_keydef(curskey_mod_key('Z', CNTRL|META)));
	test("C-M-\\",      curskey_get_keydef(curskey_mod_key('\\',CNTRL|META)));
	test("C-M-]",       curskey_get_keydef(curskey_mod_key(']', CNTRL|META)));
	test("C-M-^",       curskey_get_keydef(curskey_mod_key('^', CNTRL|META)));
	test("C-M-_",       curskey_get_keydef(curskey_mod_key('_', CNTRL|META)));

	// special keynames
	test("SPACE",       curskey_get_keydef(KEY_SPACE));
	test("BACKSPACE",   curskey_get_keydef(KEY_BACKSPACE));
	test("TAB",         curskey_get_keydef(KEY_TAB));
	test("RETURN",      curskey_get_keydef(KEY_RETURN));
	test("ESCAPE",      curskey_get_keydef(KEY_ESCAPE));

	test("M-SPACE",     curskey_get_keydef(curskey_mod_key(KEY_SPACE,     META)));
	test("M-BACKSPACE", curskey_get_keydef(curskey_mod_key(KEY_BACKSPACE, META)));
	test("M-TAB",       curskey_get_keydef(curskey_mod_key(KEY_TAB,       META)));
	test("M-RETURN",    curskey_get_keydef(curskey_mod_key(KEY_RETURN,    META)));
	test("M-ESCAPE",    curskey_get_keydef(curskey_mod_key(KEY_ESCAPE,    META)));

	test("C-SPACE",     curskey_get_keydef(curskey_mod_key(KEY_SPACE,     CNTRL)));
//  test(NULL, TODO     curskey_get_keydef(curskey_mod_key(KEY_BACKSPACE, CNTRL)));
	test(NULL,          curskey_get_keydef(curskey_mod_key(KEY_TAB,       CNTRL)));
	test(NULL,          curskey_get_keydef(curskey_mod_key(KEY_RETURN,    CNTRL)));
	test(NULL,          curskey_get_keydef(curskey_mod_key(KEY_ESCAPE,    CNTRL)));

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
