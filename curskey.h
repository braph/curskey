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
 *   - Parse attributes and colors
 *	 - Parse key definitions into curses keycodes returned by getch()
 *	 - Get the string representation of a curses keycode
 *
 * Following keys are supported:
 *	 - Curses special keys (HOME, END, LEFT, F1, ...)
 *	 - Bindings with control-key (C-x, ^x)
 *	 - Bindings with meta/alt-key (M-x, A-x)
 *
 * Usage:
 * \code{.c}
 *       initscr(); // Has to be called!
 *       if (curskey_init() == OK) {
 *       	...
 *       	curskey_destroy();
 *      }
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
#define KEY_DEL        127
#define KEY_ESCAPE     27
#define KEY_INSERT     KEY_IC
#define KEY_DELETE     KEY_DC
#define KEY_PAGEUP     KEY_PPAGE
#define KEY_PAGEDOWN   KEY_NPAGE
/// @}

/// \defgroup MODIFIER Modifiers
/// @{
#define CURSKEY_MOD_CNTRL   1U
#define CURSKEY_MOD_META    2U
#define CURSKEY_MOD_ALT     2U
/// @}

/// Holds the character that should be interpreted as **RETURN**.
/// Depending on whether nl() or nonl() was called this may be either '\\n' or '\r'.
/// It defaults to '\\n'.
extern int KEY_RETURN;

#define CURSKEY_MAX_HELPER(A,B) ((A) > (B) ? (A) : (B))
#define CURSKEY_KEY_MAX         CURSKEY_MAX_HELPER(KEY_MAX, CURSKEY_META_START + CURSKEY_META_RANGE)

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
 * @brief Destroy curskey.
 */
void curskey_destroy() CURSES_LIB_NOEXCEPT;

/**
 * @brief Return the keycode for a key with modifiers applied.
 *
 * Available modifiers are:
 * 	- **CURSKEY_MOD_META** / **CURSKEY_MOD_ALT**
 * 	- **CURSKEY_MOD_CNTRL**
 *
 * See also the macros curskey_meta_key(), curskey_cntrl_key().
 *
 * @note   This is implemented as a macro since since it shall be usable
 *         as a constant expression
 *
 * @return Keycode or **ERR** if the modifiers cannot be applied to this key.
 */
#define curskey_mod_key(KEY, MODIFIERS) \
(                                                                             \
	((MODIFIERS) == 0) ? (                                                    \
		KEY                                                                   \
	)                                                                         \
	:((MODIFIERS) == CURSKEY_MOD_CNTRL) ? (                                   \
		((KEY >= 'A' && KEY <= '_') || (KEY >= 'a' && KEY <= 'z') || KEY == ' ') ? ( \
			KEY % 32                                                          \
		) : (                                                                 \
			ERR                                                               \
		)                                                                     \
	)                                                                         \
	:((MODIFIERS) == CURSKEY_MOD_META) ? (                                    \
		(KEY >= 0 && KEY <= CURSKEY_META_RANGE) ? (                           \
			KEY + CURSKEY_META_START                                          \
		) : (                                                                 \
			ERR                                                               \
		)                                                                     \
	)                                                                         \
	:((MODIFIERS) == (CURSKEY_MOD_CNTRL|CURSKEY_MOD_META)) ? (                \
		((KEY >= 'A' && KEY <= '_') || (KEY >= 'a' && KEY <= 'z') || KEY == ' ') ? ( \
			KEY % 32 + CURSKEY_META_START                                     \
		) : (                                                                 \
			ERR                                                               \
		)                                                                     \
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

#define curskey_meta_key(KEY) \
	curskey_mod_key(KEY, CURSKEY_MOD_META)

#define curskey_cntrl_key(KEY) \
	curskey_mod_key(KEY, CURSKEY_MOD_CNTRL)

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
 * @return The key definition or \b NULL on error
 */
const char* curskey_get_keydef(int keycode) CURSES_LIB_NOEXCEPT;

/**
 * @brief Defines meta escape sequences in curses.
 *
 * @return **OK** if meta keys are available, **ERR** otherwise.
 */
int curskey_define_meta_keys() CURSES_LIB_NOEXCEPT;

/* Helper functions */

/**
 * @brief Translate the name of a curses KEY_ constant to its value.
 * 	"KEY_DOWN" -> 258
 *
 * @return Keycode or **ERR** on failure.
 */
//int curskey_keycode(const char *keyname) CURSES_LIB_NOEXCEPT;

/**
 * @brief Like curses keyname(), translates the value of a KEY_ constant to its name,
 * but strips leading "KEY_" and parentheses ("KEY_F(...)") off.
 *
 * @note This function is not thread-safe.
 *
 * @return The name of the key or **NULL** on failure.
 */
//const char* curskey_keyname(int keycode) CURSES_LIB_NOEXCEPT;

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
 * C++ std::string overloads ==================================================
 * ==========================================================================*/

#ifdef __cplusplus
#include <string>
static inline short curses_color_parse(const std::string& color)
  CURSES_LIB_NOEXCEPT
{ return curses_color_parse(color.c_str()); }

static inline unsigned int curses_attr_parse(const std::string& attribute)
  CURSES_LIB_NOEXCEPT
{ return curses_color_parse(attribute.c_str()); }

static inline int curskey_parse(const std::string& keydef)
  CURSES_LIB_NOEXCEPT
{ return curskey_parse(keydef.c_str()); }
#endif

#endif // CURSKEY_H_
