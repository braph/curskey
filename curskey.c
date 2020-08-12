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
#include <assert.h>

#ifdef __cplusplus
#define _const_cast(TYPE, VALUE) const_cast<TYPE>(VALUE)
#define _reinterpret_cast(TYPE, VALUE) reinterpret_cast<TYPE>(VALUE)
#define _malloc(TYPE, SIZE) _reinterpret_cast(
#else
#define _const_cast(TYPE, VALUE) (TYPE)(VALUE)
#define _reinterpret_cast(TYPE, VALUE) (TYPE)(VALUE)
#define _malloc(TYPE, SIZE) malloc(SIZE)
#endif

struct curskey_key {
	char *keyname;
	int keycode;
};

int KEY_RETURN = '\n';
int CURSKEY_META_START = 0;
int CURSKEY_KEY_MAX = KEY_MAX;
static int curskey_keynames_size = 0;
static struct curskey_key *curskey_keynames = NULL;

// Names for non-printable/whitespace characters
// and aliases for existing keys
static const struct curskey_key curskey_aliases[] = {
	// Keep this sorted by `keyname`
	{ _const_cast(char*, "DEL"),      KEY_DEL    },
	{ _const_cast(char*, "DELETE"),   KEY_DC     },
	{ _const_cast(char*, "ESCAPE"),   KEY_ESCAPE },
	{ _const_cast(char*, "INSERT"),   KEY_IC     },
	{ _const_cast(char*, "PAGEDOWN"), KEY_NPAGE  },
	{ _const_cast(char*, "PAGEUP"),   KEY_PPAGE  },
	{ _const_cast(char*, "SPACE"),    KEY_SPACE  },
	{ _const_cast(char*, "TAB"),      KEY_TAB    }
};
#define ALIASES_SIZE ((int) (sizeof(curskey_aliases) / sizeof(curskey_aliases[0])))

// Buffer that is used by the functions
static char buffer[256];

#define STARTSWITH_KEY(S) ( \
	(name[0] == 'K' || name[0] == 'k') && \
	(name[1] == 'E' || name[1] == 'e') && \
	(name[2] == 'Y' || name[2] == 'y') && \
	(name[3] == '_'))

#define IS_CONTROL(S) ( \
	((S[0] == '^') && S[1] != '\0') || ((S[0] == 'C' || S[0] == 'c') && S[1] == '-'))

#define IS_META(S) ( \
	(S[0] == 'M' || S[0] == 'm' || S[0] == 'A' || S[0] == 'a') && S[1] == '-')

#define IS_SHIFT(S) ( \
	(S[0] == 'S' || S[0] == 's') && S[1] == '-')

static int curskey_key_cmp(const void *a, const void *b) {
	return strcmp(((struct curskey_key*) a)->keyname,
			((struct curskey_key*) b)->keyname);
}

static int curskey_find(const struct curskey_key *table, int size, const char *name) {
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

const char* curskey_keyname(int keycode) {
	int i;

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

	for (i = 0; i < ALIASES_SIZE; ++i)
		if (keycode == curskey_aliases[i].keycode)
			return curskey_aliases[i].keyname;

	for (i = 0; i < curskey_keynames_size; ++i)
		if (keycode == curskey_keynames[i].keycode)
			return curskey_keynames[i].keyname;

	return NULL;
}

int curskey_keycode(const char *name)
{
	int i;

	if (! name)
		return ERR;

	if (STARTSWITH_KEY(name))
		name += 4;

	if (name[0] == 'F' || name[0] == 'f') {
		i = (name[1] == '(' ? 2 : 1);

		if (name[i] >= '0' && name[i] <= '9') {
			i = atoi(name + i);
			if (i >= 1 && i <= 63)
				return KEY_F(i);
		}
	}

	if (! strcasecmp(name, "RETURN"))
		return KEY_RETURN;

	i = curskey_find(curskey_aliases, ALIASES_SIZE, name);
	if (i != ERR)
		return i;

	return curskey_find(curskey_keynames, curskey_keynames_size, name);
}

static void free_ncurses_keynames() {
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
	char	*name;
	struct curskey_key *tmp;

	free_ncurses_keynames();
	curskey_keynames = (struct curskey_key*) malloc((KEY_MAX - KEY_MIN) * sizeof(struct curskey_key));
	if (!curskey_keynames)
		return ERR;

	for (int key = KEY_MIN; key != KEY_MAX; ++key) {
		name = (char*) keyname(key);

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

	tmp = (struct curskey_key*) realloc(curskey_keynames, curskey_keynames_size * sizeof(struct curskey_key));
	if (!tmp)
		goto ERROR;
	curskey_keynames = tmp;

	qsort(curskey_keynames, curskey_keynames_size, sizeof(struct curskey_key), curskey_key_cmp);

	return OK;

ERROR:
	free_ncurses_keynames();
	return ERR;
}

int curskey_define_meta_keys(int meta_start) {
#ifdef NCURSES_VERSION
	CURSKEY_META_START = meta_start;

	int ch;
	int curs_keycode = CURSKEY_META_START;
	char key_sequence[3] = "\e ";

	for (ch = 0; ch <= CURSKEY_META_END_CHARACTERS; ++ch) {
		key_sequence[1] = ch;
		define_key(key_sequence, curs_keycode);
		++curs_keycode;
	}

	CURSKEY_KEY_MAX = CURSKEY_META_START + CURSKEY_META_END_CHARACTERS;
	return OK;
#endif
	return ERR;
}

int curskey_mod_key(int key, unsigned int modifiers) {
	if (modifiers & CURSKEY_MOD_CNTRL) {
		if ((key >= 'A' && key <= '_') || (key >= 'a' && key <= 'z') || key == ' ')
			key = key % 32;
		else
			return ERR;
	}

	if (modifiers & CURSKEY_MOD_META) {
		if (CURSKEY_META_START && (key >= 0 && key <= CURSKEY_META_END_CHARACTERS))
			key = CURSKEY_META_START + key;
		else
			return ERR;
	}

	return key;
}

int curskey_unmod_key(int key, unsigned int* modifiers)
{
	unsigned int null_store;
	if (!modifiers)
		modifiers = &null_store;

	*modifiers = 0;

	if (key < 0)
		return ERR;

	/*
	if (key >= KEY_MIN && key <= KEY_MAX)
		return key;
	*/

	if (CURSKEY_CAN_META &&
			key >= CURSKEY_META_START &&
			key <= CURSKEY_META_START + CURSKEY_META_END_CHARACTERS)
	{
		key = key - CURSKEY_META_START;
		*modifiers |= CURSKEY_MOD_META;
	}

	if (key < ' ' &&
		// We do not want C-I for TAB, etc...
		key != KEY_ESCAPE &&
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
{
	unsigned int mod;
	char *s = buffer;

	keycode = curskey_unmod_key(keycode, &mod);

	/*
	if (keycode == ERR)
		return NULL;

	if (keycode >= KEY_MIN && keycode <= KEY_MAX)
		return curskey_keyname(keycode);
	*/

	if (mod & CURSKEY_MOD_CNTRL) {
		*s++ = 'C';
		*s++ = '-';
	}

	if (mod & CURSKEY_MOD_META) {
		*s++ = 'M';
		*s++ = '-';
	}

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

int curskey_parse(const char *def) {
	int c;
	unsigned int mod = 0;
	assert(def && "def is NULL");

	for (;;) {
		if (def[0] == '^' && def[1] != '\0') {
			++def;
			mod |= CURSKEY_MOD_CNTRL;
		}
		else if ((def[0] == 'C' || def[0] == 'c') && def[1] == '-') {
			def += 2;
			mod |= CURSKEY_MOD_CNTRL;
		}
		else if (IS_META(def)) {
			if (! CURSKEY_CAN_META)
				return ERR;
			def += 2;
			mod |= CURSKEY_MOD_ALT;
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

int curskey_init() {
	keypad(stdscr, TRUE);
	return create_ncurses_keynames();
}

void curskey_destroy() {
	free_ncurses_keynames();
}

// UNUSED CODE {{{
#if 0
/*
 * Return the "normalized" keyname, without preceding "KEY_"
 * and parentheses of function keys removed:
 *	 KEY_HOME -> HOME
 *	 HOME	  -> HOME
 *	 KEY_F(1) -> F1
 *
 * String will be truncated to 64 characters.
 * Returned string must be free()d.
 */
char *curskey_normalize(const char *name)
{
	char normalized[64];
	int i = 0;

	if (! name)
		return NULL;

	if (STARTSWITH_KEY(name))
		name += 4;

	if (name[0] == 'F' || name[0] == 'f') {
		for (; *name; ++name) {
			if (*name == '(' || *name == ')') {
				// ignore
			}
			else if (i < sizeof(normalized) - 1) {
				normalized[i++] = *name;
			}
		}
		normalized[i] = '\0';
		return strdup(normalized);
	}
	else {
		return strdup(name);
	}
}
#endif
// }}} UNUSED CODE
