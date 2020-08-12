/*
 * curskey.h - parse keybindings in ncurses based applications
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

/**
 * @file   curskey.h
 * @author Benjamin Abendroth
 * @date   03 Aug 2020
 * @brief  Small library for handling/parsing keybindings in ncurses based
 *         terminal applications.
 *
 * This library allows you to
 *	 - Parse key definitions into ncurses keycodes returned by getch()
 *	 - Get the string representation of a ncurses keycode
 *
 * Following keys are supported:
 *	 - Ncurses special keys (HOME, END, LEFT, F1, ...)
 *	 - Bindings with control-key (C-x, ^x)
 *	 - Bindings with meta/alt-key (M-x, A-x)
 *
 * Usage:
 *   initscr(); // Has to be called!
 *   if (curskey_init() == OK) {
 *   	...
 *   	curskey_destroy();
 *  }
 */

#ifndef CURSKEY_H_INCLUDED
#define CURSKEY_H_INCLUDED

#include <ncurses.h>

/* Additional KEY_ constants */
#define KEY_SPACE      ' '
#define KEY_TAB        '\t'
#define KEY_DEL        127
#define KEY_ESCAPE     27
#define KEY_INSERT     KEY_IC
#define KEY_DELETE     KEY_DC
#define KEY_PAGEUP     KEY_PPAGE
#define KEY_PAGEDOWN   KEY_NPAGE
/**
 * This variable holds the character that should be interpreted as RETURN.
 * Depending on whether nl() or nonl() was called this may be either '\n' or '\r'.
 * It defaults to '\n'.
 */
extern int KEY_RETURN;

/* Modifiers */
#define CURSKEY_MOD_CNTRL   1U
#define CURSKEY_MOD_META    2U
#define CURSKEY_MOD_ALT     2U

/**
 * @brief The starting keycode for enumerating meta/alt key combinations
 */
extern int CURSKEY_META_START;

/* Defines the range of characters which should be "meta-able" */
#define CURSKEY_META_END_CHARACTERS 127

/* Macro for checking if meta keys are available */
#define CURSKEY_CAN_META (CURSKEY_META_START+0)

// Holds the maximum keycode used by curskey
extern int CURSKEY_KEY_MAX;

/* Main functions */

/**
 * @brief Initialize curskey.
 * @return \bOK on success, \bERR on failure.
 */
int curskey_init();

/**
 * @brief Destroy curskey.
 */
void curskey_destroy();

/**
 * @brief Return the keycode for a key with modifiers applied.
 *
 * Available modifiers are:
 * 	- CURSKEY_MOD_META / CURSKEY_MOD_ALT
 * 	- CURSKEY_MOD_CNTRL
 *
 * See also the macros curskey_meta_key(), curskey_cntrl_key().
 *
 * @return ERR if the modifiers cannot be applied to this key.
 */
int curskey_mod_key(int key, unsigned int modifiers);

/**
 * @brief The opposite of curskey_mod_key.
 *
 * Stores modifier mask in `modifiers` if it is not NULL.
 *
 * @return The keycode with modifiers stripped of or ERR if the key is invalid.
 */
int curskey_unmod_key(int key, unsigned int *modifiers);

#define curskey_meta_key(KEY) \
	curskey_mod_key(KEY, CURSKEY_MOD_META)

#define curskey_cntrl_key(KEY) \
	curskey_mod_key(KEY, CURSKEY_MOD_CNTRL)

/**
 * @brief Return the ncurses keycode for a key definition.
 *
 * Key definition may be:
 *	- Single character (a, z, ...)
 *	- Character with control-modifier (^x, C-x, c-x, ...)
 *	- Character with meta/alt-modifier (M-x, m-x, A-x, a-x, ...)
 *	- Character with both modifiers (C-M-x, M-C-x, M-^x, ...)
 *	- Curses keyname, no modifiers allowed (KEY_HOME, HOME, F1, F(1), ...)
 *
 * Returns ERR if either
 * 	- The key definition is NULL or empty
 * 	- The key could not be found ("KEY_FOO")
 * 	- The key combination is generally invalid ("C-TAB", "C-RETURN")
 * 	- The key is invalid because of compile time options (the
 * 		`define_key()` function was not available.)
 */
int curskey_parse(const char *keydef);

/**
 * @brief Return key definition for a ncurses keycode.
 *
 * The returned string is of the format "[C-][M-]KEY".
 *
 * Returns NULL on failure.
 *
 * This function is not thread-safe.
 */
const char* curskey_get_keydef(int keycode);

/**
 * @brief Defines meta escape sequences in ncurses.
 *
 * @return OK if meta keys are available, ERR otherwise.
 */
int curskey_define_meta_keys(int meta_start);

/* Helper functions */

/**
 * @brief Translate the name of a ncurses KEY_ constant to its value.
 * 	"KEY_DOWN" -> 258
 *
 * @return ERR on failure.
 */
int curskey_keycode(const char *keyname);

/**
 * @brief Like ncurses keyname(), translates the value of a KEY_ constant to its name,
 * but strips leading "KEY_" and parentheses ("KEY_F(...)") off.
 *
 * @return The name of the key or NULL on failure.
 *
 * This function is not thread-safe.
 */
const char* curskey_keyname(int keycode);

#endif
