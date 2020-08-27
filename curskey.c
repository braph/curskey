/*
 * curskey.c - parse keybindings in ncurses based applications
 * Copyright (C) 2017 Benjamin Abendroth
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "curskey.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#ifdef __cplusplus
#define CONST_CAST(TYPE, VALUE)       const_cast<TYPE>(VALUE)
#define STATIC_CAST(TYPE, VALUE)      static_cast<TYPE>(VALUE)
#define REINTERPRET_CAST(TYPE, VALUE) reinterpret_cast<TYPE>(VALUE)
#define MALLOC(TYPE, SIZE)            STATIC_CAST(TYPE, malloc(SIZE))
#define REALLOC(TYPE, PTR, SIZE)      STATIC_CAST(TYPE, realloc(PTR, SIZE))
#else
#define CONST_CAST(TYPE, VALUE)       ((TYPE)(VALUE))
#define STATIC_CAST(TYPE, VALUE)      ((TYPE)(VALUE))
#define REINTERPRET_CAST(TYPE, VALUE) ((TYPE)(VALUE))
#define MALLOC(TYPE, SIZE)            malloc(SIZE)
#define REALLOC(TYPE, PTR, SIZE)      realloc(PTR, SIZE)
#endif

#define UPPER(CHAR) (CHAR & ~0x20)
static void define_xterm_keys()          CURSES_LIB_NOEXCEPT;
static void define_rxvt_arrow(char, int) CURSES_LIB_NOEXCEPT;
static void define_rxvt_key(char, int)   CURSES_LIB_NOEXCEPT;
static void define_rxvt_func_keys()      CURSES_LIB_NOEXCEPT;

struct curskey_key {
	char *keyname;
	int keycode;
};

int KEY_RETURN = '\n';
static int curskey_keynames_size = 0;
static struct curskey_key *curskey_keynames = NULL;

// This provides keynames for non-printable/whitespace characters.
// It also provides aliased named for existing key symbols.
static const struct curskey_key curskey_aliases[] = {
	// Keep this sorted by `keyname`
	{ CONST_CAST(char*, "DELETE"),    KEY_DC     },
	{ CONST_CAST(char*, "ESCAPE"),    KEY_ESCAPE },
	{ CONST_CAST(char*, "INSERT"),    KEY_IC     },
	{ CONST_CAST(char*, "PAGEDOWN"),  KEY_NPAGE  },
	{ CONST_CAST(char*, "PAGEUP"),    KEY_PPAGE  },
	{ CONST_CAST(char*, "SPACE"),     KEY_SPACE  },
	{ CONST_CAST(char*, "TAB"),       KEY_TAB    }
};
#define ALIASES_SIZE  STATIC_CAST(int, sizeof(curskey_aliases) / sizeof(*curskey_aliases))

// Buffer that is used by the functions
static char buffer[256];

#define STARTSWITH_KEY(S) ( \
	(UPPER(S[0]) == 'K') && \
	(UPPER(S[1]) == 'E') && \
	(UPPER(S[2]) == 'Y') && \
	(S[3] == '_'))

static int curskey_key_cmp(const void *a, const void *b)
	CURSES_LIB_NOEXCEPT
{
	return strcmp(
		STATIC_CAST(const struct curskey_key*, a)->keyname,
		STATIC_CAST(const struct curskey_key*, b)->keyname);
}

static int curskey_find(const struct curskey_key *table, int size, const char *name)
	CURSES_LIB_NOEXCEPT
{
	int start = 0;
	int end = size;
	int i;
	int cmp;

	while (1) {
		i = (start+end) / 2;
		cmp = strcasecmp(name, table[i].keyname);

		if (cmp == 0)
			return table[i].keycode;
		else if (end == start + 1)
			return ERR;
		else if (cmp > 0)
			start = i;
		else
			end = i;
	}
}

/// Like the original keyname() function.
/// Translates the value of a KEY_ constant to its name,
/// but strips leading "KEY_" and parentheses ("KEY_F(...)") off.
const char* curskey_keyname(int keycode)
	CURSES_LIB_NOEXCEPT
{
	int i;
	static char buffer[8];

	if (keycode >= KEY_F(1) && keycode <= KEY_F(63)) {
		i = keycode - KEY_F(0);
		buffer[0] = 'F';
		if (i <= 9) {
			buffer[1] = '0' + i;
			buffer[2] = '\0';
		} else {
			buffer[1] = '0' + (i / 10);
			buffer[2] = '0' + (i % 10);
			buffer[3] = '\0';
		}
		return buffer;
	}

	if (keycode == KEY_RETURN)
		return "RETURN";

	if (keycode == 127)
		return "BACKSPACE";

	for (i = 0; i < ALIASES_SIZE; ++i)
		if (keycode == curskey_aliases[i].keycode)
			return curskey_aliases[i].keyname;

	for (i = 0; i < curskey_keynames_size; ++i)
		if (keycode == curskey_keynames[i].keycode)
			return curskey_keynames[i].keyname;

	return NULL;
}

/// Translate the name of a curses KEY_ constant to its value.
static int curskey_keycode(const char *name)
	CURSES_LIB_NOEXCEPT
{
	int i;

	if (STARTSWITH_KEY(name))
		name += 4;

	if (! strcasecmp(name, "RETURN"))
		return KEY_RETURN;

	i = curskey_find(curskey_aliases, ALIASES_SIZE, name);
	if (i != ERR)
		return i;

	i = curskey_find(curskey_keynames, curskey_keynames_size, name);
	if (i != ERR)
		return i;

	if (UPPER(name[0]) == 'F') {
		name += (name[1] == '(' ? 2 : 1);

		if (name[0] >= '0' && name[0] <= '9') {
			i = strtoimax(name, NULL, 10);
			if (i >= 1 && i <= 63)
				return KEY_F(i);
		}
	}

	return ERR;
}

static void free_ncurses_keynames()
	CURSES_LIB_NOEXCEPT
{
	if (curskey_keynames) {
		while (curskey_keynames_size)
			free(curskey_keynames[--curskey_keynames_size].keyname);
		free(curskey_keynames);
		curskey_keynames = NULL;
	}
}

/**
 * Create the list of ncurses KEY_ constants.
 * Returns OK on success, ERR on failure.
 */
int create_ncurses_keynames() {
	char *name;
	void *tmp;

	free_ncurses_keynames();
	curskey_keynames = MALLOC(struct curskey_key*, (KEY_MAX - KEY_MIN) * sizeof(struct curskey_key));
	if (!curskey_keynames)
		return ERR;

	for (int key = KEY_MIN; key != KEY_MAX; ++key) {
		name = CONST_CAST(char*, keyname(key));

		if (!name || !STARTSWITH_KEY(name))
			continue;

		name += 4;
		if (name[0] == 'F' && name[1] == '(')
			continue; // ignore KEY_F(1), ...

		name = strdup(name);
		if (! name)
			goto ERROR;

		curskey_keynames[curskey_keynames_size].keycode = key;
		curskey_keynames[curskey_keynames_size].keyname = name;
		++curskey_keynames_size;
	}

	tmp = realloc(curskey_keynames, curskey_keynames_size * sizeof(struct curskey_key));
	if (!tmp)
		goto ERROR;
	curskey_keynames = STATIC_CAST(struct curskey_key*, tmp);

	qsort(curskey_keynames, curskey_keynames_size, sizeof(struct curskey_key), curskey_key_cmp);

	return OK;

ERROR:
	free_ncurses_keynames();
	return ERR;
}

#if 0
int curskey_mod_key(int key, unsigned int modifiers)
	CURSES_LIB_NOEXCEPT
{
	if (modifiers & CURSKEY_MOD_CNTRL) {
		if ((key >= 'A' && key <= '_') || (key >= 'a' && key <= 'z') || key == ' ')
			key = key % 32;
		else
			return ERR;
	}

	if (modifiers & CURSKEY_MOD_META) {
		if (CURSKEY_META_START && (key >= 0 && key <= CURSKEY_META_RANGE))
			key = CURSKEY_META_START + key;
		else
			return ERR;
	}

	return key;
}
#endif

int curskey_unmod_key(int key, unsigned int* modifiers)
	CURSES_LIB_NOEXCEPT
{
	unsigned int null_store;
	if (!modifiers)
		modifiers = &null_store;

	*modifiers = key & (CURSKEY_MOD_SHIFT| CURSKEY_MOD_ALT|CURSKEY_MOD_CNTRL);
	key &= ~(CURSKEY_MOD_SHIFT|CURSKEY_MOD_ALT|CURSKEY_MOD_CNTRL);

	if (key < 0)
		return ERR;

	if (key >= CURSKEY_META_START && key <= CURSKEY_META_START + CURSKEY_META_RANGE)
	{
		key = key - CURSKEY_META_START;
		*modifiers |= CURSKEY_MOD_META;
	}

	if (key < ' ' &&
		key != KEY_ESCAPE && // We do not want C-I for TAB, etc...
		key != KEY_TAB &&
		key != KEY_RETURN) {
		if (key == 0)
			key = ' ';
		else
			key += 'A' - 1;
		*modifiers |= CURSKEY_MOD_CNTRL;
	}

	return key;
}

const char *curskey_get_keydef(int keycode)
	CURSES_LIB_NOEXCEPT
{
	unsigned int mod;
	keycode = curskey_unmod_key(keycode, &mod);

	char *s = buffer;
	if (mod & CURSKEY_MOD_META)  { *s++ = 'M'; *s++ = '-'; }
	if (mod & CURSKEY_MOD_SHIFT) { *s++ = 'S'; *s++ = '-'; }
	if (mod & CURSKEY_MOD_CNTRL) { *s++ = 'C'; *s++ = '-'; }

	const char* name = curskey_keyname(keycode);
	if (name)
		strcpy(s, name);
	else if (keycode > 32 && keycode < 127) {
		*s++ = keycode;
		*s = '\0';
	}
	else
		return NULL;

	return buffer;
}

#define IS_CARET(S) \
	(S[0] == '^' && S[1] != '\0')

#define IS_CONTROL(S) \
	(UPPER(S[0]) == 'C' && S[1] == '-')

#define IS_META(S) \
	((UPPER(S[0]) == 'M' || UPPER(S[0]) == 'A') && S[1] == '-')

#define IS_SHIFT(S) \
	(UPPER(S[0]) == 'S' && S[1] == '-')

int curskey_parse(const char *def)
	CURSES_LIB_NOEXCEPT
{
	int c;
	unsigned int mod = 0;

	for (;;) {
		if (IS_CARET(def)) {
			++def;
			mod |= CURSKEY_MOD_CNTRL;
		}
		else if (IS_CONTROL(def)) {
			def += 2;
			mod |= CURSKEY_MOD_CNTRL;
		}
		else if (IS_META(def)) {
			def += 2;
			mod |= CURSKEY_MOD_ALT;
		}
		else if (IS_SHIFT(def)) {
			def += 2;
			mod |= CURSKEY_MOD_SHIFT;
		}
		else
			break;
	}

	if (*def == '\0')
		return ERR;
	else if (*(def+1) == '\0')
		c = *def;
	else
		c = curskey_keycode(def);

	return curskey_mod_key(c, mod);
}

int curskey_wgetch(WINDOW* win)
	CURSES_LIB_NOEXCEPT
{
	int ch = wgetch(win);
	if (ch == KEY_ESCAPE) {
		//nodelay(win, TRUE);
		wtimeout(win, 0);
		int ch2 = wgetch(win);
		wtimeout(win, -1);
		//nodelay(win, FALSE);
		if (ch2 == ERR)
			return ch; // KEY_ESCAPE
		else
			return curskey_mod_key(ch2, CURSKEY_MOD_META);
	}

	return ch;
}

int curskey_init()
	CURSES_LIB_NOEXCEPT
{
	keypad(stdscr, TRUE);

	//define_key("\x57", KEY_BACKSPACE); // 127 TODO ...

	define_xterm_keys();

	define_rxvt_arrow('A', KEY_UP);
	define_rxvt_arrow('B', KEY_DOWN);
	define_rxvt_arrow('C', KEY_RIGHT);
	define_rxvt_arrow('D', KEY_LEFT);

	define_rxvt_key('2', KEY_IC);
	define_rxvt_key('3', KEY_DC);
	define_rxvt_key('5', KEY_PPAGE);
	define_rxvt_key('6', KEY_NPAGE);
	define_rxvt_key('7', KEY_HOME);
	define_rxvt_key('8', KEY_END);

	define_rxvt_func_keys();

	return create_ncurses_keynames();
}

void curskey_destroy()
	CURSES_LIB_NOEXCEPT
{
	free_ncurses_keynames();
}

/* ============================================================================
 * Color functions ============================================================
 * ==========================================================================*/

short curses_color_parse(const char* s)
	CURSES_LIB_NOEXCEPT
{
	if (!strcmp(s, "default"))  return -1;
	if (!strcmp(s, "black"))    return COLOR_BLACK;
	if (!strcmp(s, "red"))      return COLOR_RED;
	if (!strcmp(s, "green"))    return COLOR_GREEN;
	if (!strcmp(s, "yellow"))   return COLOR_YELLOW;
	if (!strcmp(s, "blue"))     return COLOR_BLUE;
	if (!strcmp(s, "magenta"))  return COLOR_MAGENTA;
	if (!strcmp(s, "cyan"))     return COLOR_CYAN;
	if (!strcmp(s, "white"))    return COLOR_WHITE;

	char *end;
	intmax_t i = strtoimax(s, &end, 10);
	if (*s && !*end && i >= -1 && i <= 255)
		return i;

	return COLOR_INVALID;
}

const char* curses_color_tostring(short color)
	CURSES_LIB_NOEXCEPT
{
	static char buf[10];
	switch (color) {
		case -1:            return "default";
		case COLOR_BLACK:   return "black";
		case COLOR_RED:     return "red";
		case COLOR_GREEN:   return "green";
		case COLOR_YELLOW:  return "yellow";
		case COLOR_BLUE:    return "blue";
		case COLOR_MAGENTA: return "magenta";
		case COLOR_CYAN:    return "cyan";
		case COLOR_WHITE:   return "white";
		default:
			{
				// TODO: handle negative numbers
				buf[9] = 0;
				char* s = &buf[9];
				while (color) {
					*--s = '0' + (color % 10);
					color /= 10;
				}
				return s;
			}
	}

	return NULL;
}

/* ============================================================================
 * Attribute functions ========================================================
 * ==========================================================================*/

unsigned int curses_attr_parse(const char* s)
	CURSES_LIB_NOEXCEPT
{
	if (!strcmp(s, "bold"))       return A_BOLD;
	if (!strcmp(s, "dim"))        return A_DIM;
	if (!strcmp(s, "blink"))      return A_BLINK;
	if (!strcmp(s, "italic"))
#ifdef A_ITALIC
		return A_ITALIC;
#else
		return A_NORMAL;
#endif
	if (!strcmp(s, "standout"))   return A_STANDOUT;
	if (!strcmp(s, "underline"))  return A_UNDERLINE;
	if (!strcmp(s, "normal"))     return A_NORMAL;
	return A_INVALID;
}

const char* curses_attr_tostring(unsigned int attribute)
	CURSES_LIB_NOEXCEPT
{
	switch (attribute) {
		case A_BOLD:      return "bold";
		case A_DIM:       return "dim";
		case A_BLINK:     return "blink";
#ifdef A_ITALIC
		case A_ITALIC:    return "italic";
#endif
		case A_STANDOUT:  return "standout";
		case A_UNDERLINE: return "underline";
		case A_NORMAL:    return "normal";
		default:          return NULL;
	}
}

/* ============================================================================
 * Create pair functions ======================================================
 * ==========================================================================*/

static int last_id = 0;
static int color_pairs[CURSES_LIB_COLORS];

#define FG_BG(FG, BG) \
	(STATIC_CAST(unsigned short, FG) | STATIC_CAST(unsigned short, BG) << 16)

int curses_create_color_pair(short fg, short bg)
	CURSES_LIB_NOEXCEPT
{
	int pair_id;
	int pair = FG_BG(fg, bg);
	for (pair_id = 1; pair_id <= last_id; ++pair_id)
		if (color_pairs[pair_id] == pair)
			return pair_id;

	if (last_id == CURSES_LIB_COLORS)
		return ERR;

	color_pairs[pair_id] = pair;
	init_pair(pair_id, fg, bg);
	return ++last_id;
}

void curses_reset_color_pairs()
	CURSES_LIB_NOEXCEPT
{
	last_id = 0;
}

/* ============================================================================
 * Key defining functions =====================================================
 * ==========================================================================*/

#ifdef NCURSES_VERSION
static inline void define_modifier_combinations(char* definition, int keysym, int modifier_pos)
	CURSES_LIB_NOEXCEPT
{
	for (int mod = 1; mod <= 7; ++mod) {
		definition[modifier_pos] = '1' + mod;
		define_key(definition, keysym
				| (mod & 1 ? CURSKEY_MOD_SHIFT : 0)
				| (mod & 2 ? CURSKEY_MOD_ALT   : 0)
				| (mod & 4 ? CURSKEY_MOD_CNTRL : 0)
		);
	}
}

static inline void define_xterm_keys()
	CURSES_LIB_NOEXCEPT
{
	const struct {
		char seq[6]; // '\1' marks placeholder for modifier
		short key;
	} codes[] = {
		{ "[6;\1~", KEY_NPAGE },
		{ "[5;\1~", KEY_PPAGE },
		{ "[4;\1~", KEY_END   },
		{ "[3;\1~", KEY_DC    },
		{ "[2;\1~", KEY_IC    },
		{ "[1;\1~", KEY_HOME  },

		{ "[1;\1A", KEY_UP    },
		{ "[1;\1B", KEY_DOWN  },
		{ "[1;\1C", KEY_RIGHT },
		{ "[1;\1D", KEY_LEFT  },

		{ "[1;\1E", KEY_B2    }, /// not quite sure
		{ "[1;\1F", KEY_END   },
		{ "[1;\1H", KEY_HOME  },

		{ "[1;\1P", KEY_F(1)  },
		{ "[1;\1Q", KEY_F(2)  },
		{ "[1;\1R", KEY_F(3)  },
		{ "[1;\1S", KEY_F(4)  },

		{{'[','1','5',';','\1','~'}, KEY_F(5)  },
		{{'[','1','7',';','\1','~'}, KEY_F(6)  },
		{{'[','1','8',';','\1','~'}, KEY_F(7)  },
		{{'[','1','9',';','\1','~'}, KEY_F(8)  },

		{{'[','2','0',';','\1','~'}, KEY_F(9)  },
		{{'[','2','1',';','\1','~'}, KEY_F(10) },
		{{'[','2','3',';','\1','~'}, KEY_F(11) },
		{{'[','2','4',';','\1','~'}, KEY_F(12) },

		// KDE's Konsole
		{ "O\1P",   KEY_F(1)  },
		{ "O\1Q",   KEY_F(2)  },
		{ "O\1R",   KEY_F(3)  },
		{ "O\1S",   KEY_F(4)  },
	};

	for (int i = sizeof(codes)/sizeof(*codes); i--;) {
		char def[8] = {'\033', 0, 0, 0, 0, 0, 0, 0};
		memcpy(def+1, codes[i].seq, 6);
		int modifier_pos = (def[2] == '\1' ? 2 : (def[4] == '\1' ? 4 : 5));
		define_modifier_combinations(def, codes[i].key, modifier_pos);
	}
}

static void define_rxvt_arrow(char c, int key)
	CURSES_LIB_NOEXCEPT
{
	char def[] = "\033\033[ ";

	def[3] = c;
	//define_key(def,     key | CURSKEY_MOD_ALT);         // \033\033[A

	def[3] = c | 0x20;
	define_key(def + 1, key | CURSKEY_MOD_SHIFT);       //     \033[a
	//define_key(def,     key | CURSKEY_MOD_SHIFT | CURSKEY_MOD_ALT); // \033\033[a

	def[2] = 'O';
	define_key(def + 1, key | CURSKEY_MOD_CNTRL);       //     \033Oa
	//define_key(def,     key | CURSKEY_MOD_CNTRL | CURSKEY_MOD_ALT); // \033\033Oa
}

static void define_rxvt_key(char c, int key)
	CURSES_LIB_NOEXCEPT
{
	char def[] = "\033\033[ ~";

	def[3] = c;
	//define_key(def,    key | CURSKEY_MOD_ALT);

	def[4] = '$';
	define_key(def+1,  key | CURSKEY_MOD_SHIFT);
	//define_key(def,    key | CURSKEY_MOD_SHIFT | CURSKEY_MOD_ALT);

	def[4] = '^';
	define_key(def+1,  key | CURSKEY_MOD_CNTRL);
	//define_key(def,    key | CURSKEY_MOD_CNTRL | CURSKEY_MOD_ALT);

	def[4] = '@';
	define_key(def+1,  key | CURSKEY_MOD_CNTRL | CURSKEY_MOD_SHIFT);
	//define_key(def,    key | CURSKEY_MOD_CNTRL | CURSKEY_MOD_SHIFT | CURSKEY_MOD_ALT);
}

void define_rxvt_func_keys()
	CURSES_LIB_NOEXCEPT
{
	char seqs[4][12*3+1] = {
		"11~12~13~14~15~17~18~19~20~21~23~24~", // [0] NONE
		"23~24~25~26~28~29~31~32~33~34~23$24$", // [1] SHIFT
		"11^12^13^14^15^17^18^19^20^21^23^24^", // [2] CNTRL
		"23^24^25^26^28^29^31^32^33^34^23@24@", // [3] SHIFT + CNTRL
	};
	char def[] = "\033\033[ffm";

	for (int m = 4; m--;) {
		for (int f = 12; f--;) {
			def[3] = seqs[m][f*3+0];
			def[4] = seqs[m][f*3+1];
			def[5] = seqs[m][f*3+2];
			int key = KEY_F(f+1);
			int mod = (m & 1 ? CURSKEY_MOD_SHIFT : 0) | (m & 2 ? CURSKEY_MOD_CNTRL : 0);
			define_key(def+1, key | mod);
			//define_key(def,   key | mod | CURSKEY_MOD_ALT);
		}
	}
}

#endif
