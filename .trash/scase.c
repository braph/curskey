
static int read_number() {
	int i = getch();
	n = i;
	n *= 10;
}

static int curses_getkey() {
	char buf[8];

	while (getch() != ERR) {
		buf += getch();
	}

#define match(C1, C2, C3, C4) \
	(C1 == '\1' || C1 == buf[0]) && \
	(C1 == '\1' || C1 == buf[0]) && \
	(C1 == '\1' || C1 == buf[0]) && \
	(C1 == '\1' || C1 == buf[0]) && \

	if (match('[', '\1', ';', '\1', '~')) {
		mod = getmod(b[3]);
		switch (b[1]) {
			case '6': key = KEY_NPAGE; break;
			case '5': key = KEY_PPAGE; break;
			case '4': key = KEY_END  ; break;
			case '3': key = KEY_DC   ; break;
			case '2': key = KEY_IC   ; break;
			case '1': key = KEY_HOME ; break;
		}
	}

	if (match('[',';','\1','\1')) {
		mod = getmod(b[2]);
		switch (b[3]) {
			case 'A': key = KEY_UP    ; break;
			case 'B': key = KEY_DOWN  ; break;
			case 'C': key = KEY_RIGHT ; break;
			case 'D': key = KEY_LEFT  ; break;
			case 'E': key = KEY_B2    ; break;
			case 'F': key = KEY_END   ; break;
			case 'H': key = KEY_HOME  ; break;
			case 'P': key = KEY_F(1)  ; break;
			case 'Q': key = KEY_F(2)  ; break;
			case 'R': key = KEY_F(3)  ; break;
			case 'S': key = KEY_F(4)  ; break;
		}
	}
	if (match('[','\1'


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

static int read_escape_sequence() {
	// We have already read ESC
	

	
	int ch1 = getch();
	if (ch1 == ERR)
		return KEY_ESCAPE;

	int ch2 = getch();
	if (ch2 == ERR)
		return curskey_mod_key(ch1, CURSKEY_MOD_META);

	switch (ch1) {


	if (ch == '[') {
		int num = read_number();

		switch (num) {
		case 

	}

}

"["
" 6;"
"   1~"
"   2~"

switch (ch[0]) {
case 27:
	switch (ch[1]) {
	case '[':
		switch (ch[2]) {
		case '6':

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
