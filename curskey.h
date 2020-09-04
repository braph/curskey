/*
 * curskey.h - parse keybindings in curses based applications
 * Copyright (C) 2020 Benjamin Abendroth
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
 * @brief  Small library for handling/parsing keybindings in curses based
 *         terminal applications.
 *
 * This library allows you to
 *   - Parse colors and attributes
 *	 - Parse key definitions into curses keycodes returned by getch()
 *	 - Get the string representation of a curses keycode
 *
 * Following keys are supported:
 *	 - Curses special keys (HOME, END, LEFT, F1, ...)
 *	 - Bindings with control-key (C-x, ^x)
 *	 - Bindings with meta/alt-key (M-x, A-x)
 *	 - Bindings with shift key
 *
 * Usage:
 * \code{.c}
 *       initscr(); // Has to be called!
 *       if (curskey_init() == OK) {
 *       	...
 *       }
 * \endcode
 */

#ifndef CURSKEY_H_
#define CURSKEY_H_

#include <ncurses.h>

/// \defgroup CONF Library configuration
/// @{
/// Defines how many color pairs can be used
#define CURSES_LIB_COLORS  256
/// The starting keycode for enumerating meta/alt key combinations
#define CURSKEY_META_START 128
/// Defines the range of characters which should be "meta-able"
#define CURSKEY_META_RANGE 127
/// @}

/// \defgroup CODES Return codes
/// @{
#define A_INVALID      0xFF
#define COLOR_INVALID -0xFF
/// @}

/// \defgroup KEYS Additional KEY_ constants
/// @{
#define KEY_SPACE      ' '
#define KEY_TAB        '\t'
#define KEY_ESCAPE     27
#define KEY_INSERT     KEY_IC
#define KEY_DELETE     KEY_DC
#define KEY_PAGEUP     KEY_PPAGE
#define KEY_PAGEDOWN   KEY_NPAGE
/// @}

/// \defgroup MODIFIER Modifiers
/// @{
#define CURSKEY_MOD_SHIFT   (1 << 9)
#define CURSKEY_MOD_META    (1 << 10)
#define CURSKEY_MOD_CTRL    (1 << 11)
/// @}

/// Holds the character that should be interpreted as **RETURN**.
/// Depending on whether nl() or nonl() was called this may be either '\\n' or '\r'.
/// It defaults to '\\n'.
extern int KEY_RETURN;

#define CURSKEY_KEY_MAX (KEY_MAX|CURSKEY_MOD_SHIFT|CURSKEY_MOD_META|CURSKEY_MOD_CTRL)

#ifdef __cplusplus
#define CURSES_LIB_NOEXCEPT noexcept
#else
#define CURSES_LIB_NOEXCEPT /* noexcept */
#endif

/* ============================================================================
 * Main functions =============================================================
 * ==========================================================================*/

/**
 * @brief Initialize curskey.
 * @return **OK** on success, **ERR** on failure
 */
int curskey_init() CURSES_LIB_NOEXCEPT;

/**
 * @brief Return the keycode for a key with modifiers applied.
 *
 * Available modifiers are:
 * 	- **CURSKEY_MOD_META**
 * 	- **CURSKEY_MOD_CTRL**
 * 	- **CURSKEY_MOD_SHIFT**
 *
 * @note   This is implemented as a macro since since it shall be usable
 *         as a constant expression (for example in switch-case).
 *
 * @return Keycode or **ERR** if the modifiers cannot be applied to this key.
 */
#define curskey_mod_key(KEY, MOD)                                             \
(                                                                             \
	((MOD) & ~(CURSKEY_MOD_CTRL|CURSKEY_MOD_META|CURSKEY_MOD_SHIFT)) ? (      \
		ERR /* Invalid modifier */                                            \
	)                                                                         \
	:(((KEY) >= 'a' && (KEY) <= 'z') || ((KEY) >= 'A' && (KEY) <= 'Z')) ? (   \
		(((KEY)                                                               \
			& (((MOD) & CURSKEY_MOD_SHIFT) ? ~0x20 : 0xFF))                   \
			% (((MOD) & CURSKEY_MOD_CTRL)  ? 32    : 0xFF))                   \
			+ (((MOD) & CURSKEY_MOD_META)  ? CURSKEY_META_START : 0)          \
	)                                                                         \
	:(((KEY) >= '[' && ((KEY) <= '_')) || (KEY) == ' ') ? (                   \
		((MOD) & CURSKEY_MOD_SHIFT) ? (                                       \
			ERR /* Shift not allowed here */                                  \
		)                                                                     \
		: ((KEY)                                                              \
			% (((MOD) & CURSKEY_MOD_CTRL) ? 32 : 0xFF))                       \
			+ (((MOD) & CURSKEY_MOD_META) ? CURSKEY_META_START : 0)           \
	)                                                                         \
	:((KEY) >= 0 && (KEY) <= CURSKEY_META_RANGE) ? (                          \
		((MOD) & (CURSKEY_MOD_SHIFT|CURSKEY_MOD_CTRL)) ? (                    \
			ERR /* Shift / Control not allowed here */                        \
		)                                                                     \
		: (KEY)                                                               \
			+ (((MOD) & CURSKEY_MOD_META)  ? CURSKEY_META_START : 0)          \
	)                                                                         \
	:((KEY) >= KEY_MIN && (KEY) <= KEY_MAX) ? (                               \
		(KEY) | (signed) (MOD) /*TODO cast*/                                  \
	)                                                                         \
	:(                                                                        \
		ERR                                                                   \
	)                                                                         \
)

/**
 * @brief The opposite of curskey_mod_key.
 *
 * Stores modifier mask in `modifiers` if it is not **NULL**.
 *
 * @return The keycode with modifiers stripped of or **ERR** if the key is invalid.
 */
int curskey_unmod_key(int key, unsigned int *modifiers) CURSES_LIB_NOEXCEPT;

/**
 * @brief Return the curses keycode for a key definition.
 *
 * Key definition may be:
 *	- Single character (a, z, ...)
 *	- Character with control-modifier (^x, C-x, c-x, ...)
 *	- Character with meta/alt-modifier (M-x, m-x, A-x, a-x, ...)
 *	- Character with both modifiers (C-M-x, M-C-x, M-^x, ...)
 *	- Curses keyname, no modifiers allowed (KEY_HOME, HOME, F1, F(1), ...)
 *
 * Returns **ERR** if either
 * 	- The key definition is NULL or empty
 * 	- The key could not be found ("KEY_FOO")
 * 	- The key combination is generally invalid ("C-TAB", "C-RETURN")
 * 	- The key is invalid because of compile time options (the
 * 		`define_key()` function was not available.)
 *
 * 	@return The curses keycode or **ERR** on error 
 */
int curskey_parse(const char *keydef) CURSES_LIB_NOEXCEPT;

/**
 * @brief Return key definition for a curses keycode.
 *
 * The returned string is of the format "[C-][M-]KEY".
 *
 * @note This function is not thread-safe.
 *
 * @return The key definition or **NULL** on error
 */
const char* curskey_get_keydef(int keycode) CURSES_LIB_NOEXCEPT;

/**
 * @brief Replacement for wgetch
 *
 * @note  This changes the timeout of the window using **wtimeout()**
 *
 * @return Keycode
 */
int curskey_wgetch(WINDOW*) CURSES_LIB_NOEXCEPT;

/**
 * @see curskey_wgetch()
 */
#define curskey_getch() curskey_wgetch(stdscr)

/* ============================================================================
 * Color functions ============================================================
 * ==========================================================================*/

/**
 * @brief Parses a color string
 *
 * Possible colors are
 *   - **black**, **red**, **green**, **yellow**, **blue**, **magenta**, **cyan**, **white** or **default**
 *   - a number between **-1** and **255**
 *
 * @return Color number or **COLOR_INVALID** on error
 */
short curses_color_parse(const char* s) CURSES_LIB_NOEXCEPT;

/**
 * @brief Get string for a curses color
 * @return string or **NULL** on error
 */
const char* curses_color_tostring(short color) CURSES_LIB_NOEXCEPT;

/* ============================================================================
 * Attribute functions ========================================================
 * ==========================================================================*/

/**
 * @brief Parse an attribute string
 *
 * Possible attributes are:
 *   **bold**, **dim**, **blink**, **italic**, **standout**, **underline** or **normal**
 *
 * @note   If the **A_ITALIC** attribute is not available at compile time,
 *         **A_NORMAL** will be returned for **italic**
 *
 * @return Curses attribute or **A_INVALID** on error
 */
unsigned int curses_attr_parse(const char* s) CURSES_LIB_NOEXCEPT;

/**
 * @brief  Get string for a curses attribute
 * @return String or **NULL** on error
 */
const char* curses_attr_tostring(unsigned int attribute) CURSES_LIB_NOEXCEPT;

/* ============================================================================
 * Create pair functions ======================================================
 * ==========================================================================*/

/**
 * @brief  Create a color pair
 * @return Color pair or **ERR** on error
 */
int curses_create_color_pair(short fg, short bg) CURSES_LIB_NOEXCEPT;

/**
 * @brief Resets the color pairs created by curses_create_color_pair()
 */
void curses_reset_color_pairs() CURSES_LIB_NOEXCEPT;

/* ============================================================================
 * C++ String overloads =======================================================
 * ==========================================================================*/

#ifdef __cplusplus
template<class String> inline short curses_color_parse(const String& color)
	CURSES_LIB_NOEXCEPT
{ return curses_color_parse(color.c_str()); }

template<class String> inline unsigned int curses_attr_parse(const String& attribute)
	CURSES_LIB_NOEXCEPT
{ return curses_color_parse(attribute.c_str()); }

template<class String> inline int curskey_parse(const String& keydef)
	CURSES_LIB_NOEXCEPT
{ return curskey_parse(keydef.c_str()); }
#endif

#endif /* CURSKEY_H_ */
