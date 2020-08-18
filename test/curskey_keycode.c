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

