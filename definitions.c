#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#define SHIFT (1 << 9)
#define ALT   (1 << 10)
#define CNTRL (1 << 11)

// CNTRL 5, ALT 3, SHIFT 2
// CNTRL+SHIFT 6, CNTRL+ALT 7
// SHIFT + ALT = 4
// ALL = 8

void define_modifier_combinations(char* keydef, int keysym, int modifier_pos) {
   for (int i = 1; i <= 7; ++i) {
      keydef[modifier_pos] = '0' + i + 1;
      define_key(keydef, keysym
        | (i & 1 ? SHIFT : 0)
        | (i & 2 ? ALT   : 0)
        | (i & 4 ? CNTRL : 0)
      );
   }
}

void define_some(char c, int key) {
   char def[] = "\033\033[ ~";

   def[3] = c;
   define_key(def, ALT | key);

   def[4] = '$';
   define_key(def+1,       SHIFT | key);
   define_key(def,   ALT | SHIFT | key);

   def[4] = '^';
   define_key(def+1,       CNTRL | key);
   define_key(def,   ALT | CNTRL | key);

   def[4] = '@';
   define_key(def+1,        CNTRL | SHIFT | key);
   define_key(def,   ALT  | CNTRL | SHIFT | key);
}

void define_urxvt_arrow(char c, int key) {
   char def[] = "\033\033[ ";

   def[3] = c;
   define_key(def, ALT   | key);                // \033\033[A

   def[3] = c | 0x20;
   define_key(def + 1, SHIFT |         key);    // \033\033[a
   define_key(def,     ALT   | SHIFT | key);    // \033\033[A

   def[2] = 'O';
   define_key(def + 1, CNTRL |         key);    // \033\03
   define_key(def,     ALT   | CNTRL | key);    // \033\03
}

void define_urxvt_fkey() {
   char def[] = "\033\033[  ~";

   char fnorml[12][4] = {"11~", "12~", "13~", "14~", "15~", "17~", "18~", "19~", "20~", "21~", "23~", "24~"};
   char fshift[12][4] = {"23~", "24~", "25~", "26~", "28~", "29~", "31~", "32~", "33~", "34~", "23$", "24$"};
   char f_s_cn[12][4] = {"23^", "24^", "25^", "26^", "28^", "29^", "31^", "32^", "33^", "34^", "23@", "24@"};

   for (int i = 0; i < 12; ++i) {
      def[3] = fnorml[i][0];
      def[4] = fnorml[i][1];
      def[5] = fnorml[i][2];
      define_key(def+1, KEY_F(i+1));
      define_key(def,   KEY_F(i+1) | ALT);

      def[5] = '^';
      define_key(def+1, KEY_F(i+1) | CNTRL);
      define_key(def,   KEY_F(i+1) | CNTRL | ALT);

      def[3] = fshift[i][0];
      def[4] = fshift[i][1];
      def[5] = fshift[i][2];
      define_key(def+1, KEY_F(i+1) | SHIFT);
      define_key(def,   KEY_F(i+1) | SHIFT | ALT);

      def[3] = f_s_cn[i][0];
      def[4] = f_s_cn[i][1];
      def[5] = f_s_cn[i][2];
      define_key(def+1, KEY_F(i+1) | SHIFT | CNTRL );
      define_key(def,   KEY_F(i+1) | SHIFT | CNTRL | ALT);
   }

#if 0
   // SHIFT  CNTRL   SHIFT|CNTRL

      define_key(def, ALT | KEY_F(i+1));
      def[5] = '^';
      define_key(def+1, CNTRL | KEY_F(i+1));

      def[5] = '~';
      def[3] = fshift[i][0];
      def[4] = fshift[i][1];
      define_key(def+1, SHIFT | KEY_F(i+1));
      define_key(def, ALT | SHIFT | KEY_F(i+1));
      def[5] = '^';
      define_key(def+1, SHIFT | CNTRL | KEY_F(i+1));
   }
#endif
}

int main() {
   initscr();
   keypad(stdscr, TRUE);

#if 0
   // 3=DC, 2=IC, 4=end, 1=home 5=up, 6down
   {
      int KEYS[] = {KEY_HOME, KEY_IC, KEY_DC, KEY_END, KEY_PPAGE, KEY_NPAGE};
      char def[] = "\033[ ; ~"; // 2 = key 4 = mod

      for (int k = 0; k <= 5; ++k) {
         def[2] = '0' + k + 1;
         define_modifier_combinations(def, KEYS[k], 4);
      }
   }
#endif

   {
      char def[] = "\033[ ; ~";
      // \033 [ <KEY> ; <MODIFIER> ;
      def[2] = '6'; define_modifier_combinations(def, KEY_NPAGE, 4);
      def[2] = '5'; define_modifier_combinations(def, KEY_PPAGE, 4);
      def[2] = '4'; define_modifier_combinations(def, KEY_END,   4);
      def[2] = '3'; define_modifier_combinations(def, KEY_DC,    4);
      def[2] = '2'; define_modifier_combinations(def, KEY_IC,    4);
      def[2] = '1'; define_modifier_combinations(def, KEY_HOME,  4);
      // \033 [ 1 ; <MODIFIER> <KEY>
      def[5] = 'A'; define_modifier_combinations(def, KEY_UP,    4);
      def[5] = 'B'; define_modifier_combinations(def, KEY_DOWN,  4);
      def[5] = 'C'; define_modifier_combinations(def, KEY_RIGHT, 4);
      def[5] = 'D'; define_modifier_combinations(def, KEY_LEFT,  4);
      def[5] = 'E'; define_modifier_combinations(def, KEY_B2,    4); // ???
      def[5] = 'F'; define_modifier_combinations(def, KEY_END,   4);
      def[5] = 'H'; define_modifier_combinations(def, KEY_HOME,  4);
      def[5] = 'P'; define_modifier_combinations(def, KEY_F(1),  4);
      def[5] = 'Q'; define_modifier_combinations(def, KEY_F(2),  4);
      def[5] = 'R'; define_modifier_combinations(def, KEY_F(3),  4);
      def[5] = 'S'; define_modifier_combinations(def, KEY_F(4),  4);
      // \033 [ 1 <KEY> ; <MODIFIER> ~
      def[4] = ';';
      def[6] = '~';
      def[3] = '5'; define_modifier_combinations(def, KEY_F(5),  5);
      def[3] = '7'; define_modifier_combinations(def, KEY_F(6),  5);
      def[3] = '8'; define_modifier_combinations(def, KEY_F(7),  5);
      def[3] = '9'; define_modifier_combinations(def, KEY_F(8),  5);
      // \033 [ 2 <KEY> ; <MODIFIER> ~
      def[2] = '2';
      def[3] = '0'; define_modifier_combinations(def, KEY_F(9),  5);
      def[3] = '1'; define_modifier_combinations(def, KEY_F(10), 5);
      def[3] = '3'; define_modifier_combinations(def, KEY_F(11), 5);
      def[3] = '4'; define_modifier_combinations(def, KEY_F(12), 5);

      // KDE's Konsole
      // \033 O <MODIFIER> <KEY>
      strcpy(def, "\033O__");
      def[3] = 'P'; define_modifier_combinations(def, KEY_F(1), 2);
      def[3] = 'Q'; define_modifier_combinations(def, KEY_F(2), 2);
      def[3] = 'R'; define_modifier_combinations(def, KEY_F(3), 2);
      def[3] = 'S'; define_modifier_combinations(def, KEY_F(4), 2);

      // URXVT
      // \033 \033 [ D
      define_urxvt_arrow('A', KEY_UP);
      define_urxvt_arrow('B', KEY_DOWN);
      define_urxvt_arrow('C', KEY_RIGHT);
      define_urxvt_arrow('D', KEY_LEFT);

      define_some('2', KEY_IC);
      define_some('3', KEY_DC);
      define_some('5', KEY_PPAGE);
      define_some('6', KEY_NPAGE);
      define_some('7', KEY_HOME);
      define_some('8', KEY_END);

      define_urxvt_fkey();

#if FOOO
      define_key("\033\033[A", ALT | KEY_UP);
      define_key("\033\033[B", ALT | KEY_DOWN);
      define_key("\033\033[C", ALT | KEY_RIGHT);
      define_key("\033\033[D", ALT | KEY_LEFT);
      define_key("\033[a", SHIFT | KEY_UP);
      define_key("\033[b", SHIFT | KEY_DOWN);
      define_key("\033[c", SHIFT | KEY_RIGHT);
      define_key("\033[d", SHIFT | KEY_LEFT);
      define_key("\033Oa", CNTRL | KEY_UP);
      define_key("\033Ob", CNTRL | KEY_DOWN);
      define_key("\033Oc", CNTRL | KEY_RIGHT);
      define_key("\033Od", CNTRL | KEY_LEFT);

      define_key("\033\033[3~", ALT   | KEY_DC);
      define_key("\033[3^",     CNTRL | KEY_DC);
#endif
   }

   for (;;) {
      int c = getch();
      int old = c;

      char resolv[] = "   ";
      if (c & SHIFT) resolv[0] = 'S';
      if (c & ALT)   resolv[1] = 'A';
      if (c & CNTRL) resolv[2] = 'C';
      //c &= ~(SHIFT|ALT|CNTRL);
      c = ((unsigned int) c) & ~(SHIFT|ALT|CNTRL);

      //printf("%d %d %d\n", KEY_DC, old ,c);
      wprintw(stdscr, "%d %s %s\n", c, resolv, keyname(c));
   }
}
