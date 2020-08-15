#include <assert.h>
#include <string.h>
#include "../curskey.h"

static int streq(const char *a, const char *b) {
	return ((!a || !b) ? a == b : !strcmp(a, b));
}

int main() {

	// ========================================================================
	// curses_attr_tostring() =================================================
	// ========================================================================

	assert(streq(curses_attr_tostring(0xFF),        NULL));
	assert(streq(curses_attr_tostring(A_BOLD),      "bold"));
	assert(streq(curses_attr_tostring(A_DIM),       "dim"));
	assert(streq(curses_attr_tostring(A_BLINK),     "blink"));
#ifdef A_ITALIC
	assert(streq(curses_attr_tostring(A_ITALIC),    "italic"));
#endif
	assert(streq(curses_attr_tostring(A_STANDOUT),  "standout"));
	assert(streq(curses_attr_tostring(A_UNDERLINE), "underline"));
	assert(streq(curses_attr_tostring(A_NORMAL),    "normal"));

	// ========================================================================
	// curses_attr_parse() ====================================================
	// ========================================================================

	assert(curses_attr_parse("")               == A_INVALID);
	assert(curses_attr_parse("bold")           == A_BOLD);
	assert(curses_attr_parse("dim")            == A_DIM);
	assert(curses_attr_parse("blink")          == A_BLINK);
#ifdef A_ITALIC
	assert(curses_attr_parse("italic")         == A_ITALIC);
#else
	assert(curses_attr_parse("italic")         == A_NORMAL);
#endif
	assert(curses_attr_parse("standout")       == A_STANDOUT);
	assert(curses_attr_parse("underline")      == A_UNDERLINE);
	assert(curses_attr_parse("normal")         == A_NORMAL);


	// ========================================================================
	// curses_color_parse() ===================================================
	// ========================================================================

	assert(curses_color_parse("default")          == -1);
	assert(curses_color_parse("white")            == COLOR_WHITE);
	assert(curses_color_parse("black")            == COLOR_BLACK);
	assert(curses_color_parse("red")              == COLOR_RED);
	assert(curses_color_parse("blue")             == COLOR_BLUE);
	assert(curses_color_parse("cyan")             == COLOR_CYAN);
	assert(curses_color_parse("green")            == COLOR_GREEN);
	assert(curses_color_parse("yellow")           == COLOR_YELLOW);
	assert(curses_color_parse("magenta")          == COLOR_MAGENTA);
	assert(curses_color_parse("123")              == 123);
	assert(curses_color_parse("no_color")         == COLOR_INVALID);
	assert(curses_color_parse("256")              == COLOR_INVALID);
	assert(curses_color_parse("-2")               == COLOR_INVALID);

	// ========================================================================
	// curses_color_tostring() ================================================
	// ========================================================================

	assert(streq(curses_color_tostring(-1)            , "default"));
	assert(streq(curses_color_tostring(COLOR_WHITE)   , "white"));
	assert(streq(curses_color_tostring(COLOR_BLACK)   , "black"));
	assert(streq(curses_color_tostring(COLOR_RED)     , "red"));
	assert(streq(curses_color_tostring(COLOR_BLUE)    , "blue"));
	assert(streq(curses_color_tostring(COLOR_CYAN)    , "cyan"));
	assert(streq(curses_color_tostring(COLOR_GREEN)   , "green"));
	assert(streq(curses_color_tostring(COLOR_YELLOW)  , "yellow"));
	assert(streq(curses_color_tostring(COLOR_MAGENTA) , "magenta"));
	assert(streq(curses_color_tostring(123)           , "123"));

	// ========================================================================
	// curses_create_color_pair() =============================================
	// ========================================================================

	assert(curses_create_color_pair(COLOR_BLUE, COLOR_BLACK) == 1);
	assert(curses_create_color_pair(COLOR_BLUE, COLOR_BLACK) == 1);
	assert(curses_create_color_pair(-1, -1)                  == 2);
	assert(curses_create_color_pair(-1, -1)                  == 2);
	assert(curses_create_color_pair(-1, COLOR_BLACK)         == 3);
	assert(curses_create_color_pair(-1, COLOR_BLACK)         == 3);
	assert(curses_create_color_pair(COLOR_BLACK, -1)         == 4);
	assert(curses_create_color_pair(COLOR_BLACK, -1)         == 4);
}

/* vim: set ts=4 sw=4 : */
