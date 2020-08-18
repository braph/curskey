// ============================================================================
// curskey_normalize() ========================================================
// ============================================================================

int main()
{
	// This function has been dropped.

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

	return 0;
}

/* vim: set ts=4 sw=4 : */
