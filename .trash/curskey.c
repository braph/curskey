#ifdef __cplusplus
#define CONST_CAST(TYPE, VALUE)       const_cast<TYPE>(VALUE)
#define REINTERPRET_CAST(TYPE, VALUE) reinterpret_cast<TYPE>(VALUE)
#define MALLOC(TYPE, SIZE)            STATIC_CAST(TYPE, malloc(SIZE))
#define REALLOC(TYPE, PTR, SIZE)      STATIC_CAST(TYPE, realloc(PTR, SIZE))
#else
#define CONST_CAST(TYPE, VALUE)       ((TYPE)(VALUE))
#define REINTERPRET_CAST(TYPE, VALUE) ((TYPE)(VALUE))
#define MALLOC(TYPE, SIZE)            malloc(SIZE)
#define REALLOC(TYPE, PTR, SIZE)      realloc(PTR, SIZE)
#endif

void define_meta_keys()
	CURSES_LIB_NOEXCEPT
{
	int ch;
	int curs_keycode = CURSKEY_META_START;
	char key_sequence[3] = "\e ";

	for (ch = 0; ch <= CURSKEY_META_RANGE; ++ch) {
		key_sequence[1] = ch;
		if (ch != 27)
			define_key(key_sequence, curs_keycode);
		++curs_keycode;
	}
}

static int curskey_keynames_size = 0;
static struct curskey_key *curskey_keynames = NULL;

static int curskey_key_cmp(const void *a, const void *b)
	CURSES_LIB_NOEXCEPT
{
	return strcmp(
		STATIC_CAST(const struct curskey_key*, a)->keyname,
		STATIC_CAST(const struct curskey_key*, b)->keyname);
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

/**
 * @brief Destroy curskey.
 *
 * This function can be called as soon as the functions
 *   - **curskey_get_keydef** and
 *   - **curskey_parse**
 * will no longer be in use.
 */
void curskey_destroy()
	CURSES_LIB_NOEXCEPT
{
	free_ncurses_keynames();
}

	for (i = 0; i < curskey_keynames_size; ++i)
		if (keycode == curskey_keynames[i].keycode)
			return curskey_keynames[i].keyname;

	for (i = KEY_MIN; i <= KEY_MAX; ++i)
		if (i < KEY_F(1) || i > KEY_F(63)) {
			const char* s = keyname(i);
			if (s && *s)
				return STARTSWITH_KEY(s) ? s + 4: s;
		}

static inline void define_rxvt_arrow_keys()
	CURSES_LIB_NOEXCEPT
{
	static const struct {
		char seq[4];
		short key;
	} codes[] = {
		{ "\033[A", KEY_UP },
		{ "\033[B", KEY_DOWN },
		{ "\033[C", KEY_RIGHT },
		{ "\033[D", KEY_LEFT },
		{ "\033[a", curskey_mod_key(KEY_UP,    CURSKEY_MOD_SHIFT) },
		{ "\033[b", curskey_mod_key(KEY_DOWN,  CURSKEY_MOD_SHIFT) },
		{ "\033[c", curskey_mod_key(KEY_RIGHT, CURSKEY_MOD_SHIFT) },
		{ "\033[d", curskey_mod_key(KEY_LEFT,  CURSKEY_MOD_SHIFT) },
		{ "\033Oa", curskey_mod_key(KEY_UP,    CURSKEY_MOD_CTRL) },
		{ "\033Ob", curskey_mod_key(KEY_DOWN,  CURSKEY_MOD_CTRL) },
		{ "\033Oc", curskey_mod_key(KEY_RIGHT, CURSKEY_MOD_CTRL) },
		{ "\033Od", curskey_mod_key(KEY_LEFT,  CURSKEY_MOD_CTRL) },
	};

	for (int i = sizeof(codes)/sizeof(*codes); i--;)
		define_key(codes[i].seq, codes[i].key);
}

#define CODES \
	X('[', 'A', KEY_UP), \
	X('[', 'B', KEY_DOWN), \
	X('[', 'C', KEY_RIGHT), \
	X('[', 'D', KEY_LEFT), \
	X('[', 'a', curskey_mod_key(KEY_UP,    CURSKEY_MOD_SHIFT)), \
	X('[', 'b', curskey_mod_key(KEY_DOWN,  CURSKEY_MOD_SHIFT)), \
	X('[', 'c', curskey_mod_key(KEY_RIGHT, CURSKEY_MOD_SHIFT)), \
	X('[', 'd', curskey_mod_key(KEY_LEFT,  CURSKEY_MOD_SHIFT)), \
	X('O', 'a', curskey_mod_key(KEY_UP,    CURSKEY_MOD_CTRL)), \
	X('O', 'b', curskey_mod_key(KEY_DOWN,  CURSKEY_MOD_CTRL)), \
	X('O', 'c', curskey_mod_key(KEY_RIGHT, CURSKEY_MOD_CTRL)), \
	X('O', 'd', curskey_mod_key(KEY_LEFT,  CURSKEY_MOD_CTRL)), \

#define X(A,B, CODE) CODE
	const short codes[12] = {CODES};
#undef X
#define X(A,B, CODE) {'\033',A,B,'\0'}
	const char seq[12][4] = {CODES};
#undef X
