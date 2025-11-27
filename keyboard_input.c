
#include "keyboard_input.h"
#include "screen.h"

/* GCC se queja de que no usamos el scancode si no le aseguramos que esa es nuestra intencion */
#define unused __attribute__((unused))

/* Estructura para mantener el estado del cursor */
typedef struct cursor_state_t {
  int row;
  int column;
  uint8_t attr;
  ca under_marker;
} cursor_state;

/* Estado global del cursor */
static cursor_state cursor = {
  .row = 0,
  .column = 0,
  .attr = C_FG_BLACK | C_BG_RED,
  .under_marker = { .c = ' ', .a = C_FG_BLACK | C_BG_BLUE }
};

/* Estado del teclado */
static int is_shift_pressed = 0;

/* Forward declarations de handlers */
static void unrecognized_scancode(uint8_t scancode);
static void toggle_shift(uint8_t);
static void move_down(uint8_t);
static void move_up(uint8_t);
static void move_left(uint8_t);
static void move_right(uint8_t);
static void new_line(uint8_t);

/* Indices para handlers especiales */
#define TOGGLE_SHIFT_IDX 1
#define MOVE_DOWN_IDX    2
#define MOVE_UP_IDX      3
#define MOVE_LEFT_IDX    4
#define MOVE_RIGHT_IDX   5
#define NEW_LINE_IDX     6

/* Array de punteros a funciones para handlers especiales */
typedef void (*scancode_handler)(uint8_t scancode);
static const scancode_handler special_scancodes[] = {
  unrecognized_scancode,
  toggle_shift,
  move_down,
  move_up,
  move_left,
  move_right,
  new_line
};

/* Informacion sobre cada scancode */
struct scancode_info {
  uint8_t main_value;     /* Letra sin shift, o '\0' si es tecla especial */
  uint8_t special_value;  /* Letra con shift, o indice en special_scancodes */
};

/* Tabla de scancodes - extraida de https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html */
static const struct scancode_info scancode_defs[256] = {
  [0x01] = { '\0', '\0' },  /* Esc */

  /* Numeros */
  [0x02] = { '1', '!' },
  [0x03] = { '2', '@' },
  [0x04] = { '3', '#' },
  [0x05] = { '4', '$' },
  [0x06] = { '5', '%' },
  [0x07] = { '6', '^' },
  [0x08] = { '7', '&' },
  [0x09] = { '8', '*' },
  [0x0a] = { '9', '(' },
  [0x0b] = { '0', ')' },
  [0x0c] = { '-', '_' },
  [0x0d] = { '=', '+' },
  [0x0e] = { '\0', MOVE_LEFT_IDX },  /* Backspace */

  /* Primera fila de letras */
  [0x0f] = { '\0', '\0' },  /* Tab */
  [0x10] = { 'q', 'Q' },
  [0x11] = { 'w', 'W' },
  [0x12] = { 'e', 'E' },
  [0x13] = { 'r', 'R' },
  [0x14] = { 't', 'T' },
  [0x15] = { 'y', 'Y' },
  [0x16] = { 'u', 'U' },
  [0x17] = { 'i', 'I' },
  [0x18] = { 'o', 'O' },
  [0x19] = { 'p', 'P' },
  [0x1a] = { '[', '{' },
  [0x1b] = { ']', '}' },

  [0x1c] = { '\0', NEW_LINE_IDX },  /* Enter */
  [0x1d] = { '\0', '\0' },          /* LCtrl */

  /* Segunda fila de letras */
  [0x1e] = { 'a', 'A' },
  [0x1f] = { 's', 'S' },
  [0x20] = { 'd', 'D' },
  [0x21] = { 'f', 'F' },
  [0x22] = { 'g', 'G' },
  [0x23] = { 'h', 'H' },
  [0x24] = { 'j', 'J' },
  [0x25] = { 'k', 'K' },
  [0x26] = { 'l', 'L' },
  [0x27] = { ';', ':' },
  [0x28] = { '\'', '"' },
  [0x29] = { '`', '~' },

  /* Shift */
  [0x2a] = { '\0', TOGGLE_SHIFT_IDX },       /* LShift */
  [0x80 | 0x2a] = { '\0', TOGGLE_SHIFT_IDX },/* LShift (UP) */

  [0x2b] = { '\\', '|' },  /* En teclado de 102 teclas */

  /* Tercera fila de letras */
  [0x2c] = { 'z', 'Z' },
  [0x2d] = { 'x', 'X' },
  [0x2e] = { 'c', 'C' },
  [0x2f] = { 'v', 'V' },
  [0x30] = { 'b', 'B' },
  [0x31] = { 'n', 'N' },
  [0x32] = { 'm', 'M' },
  [0x33] = { ',', '<' },
  [0x34] = { '.', '>' },
  [0x35] = { '/', '?' },
  [0x36] = { '\0', '\0' },  /* RShift */

  [0x37] = { '\0', '\0' },  /* Keypad-* o PrtScn */
  [0x38] = { '\0', '\0' },  /* LAlt */
  [0x39] = { ' ', ' ' },    /* Space bar */
  [0x3a] = { '\0', TOGGLE_SHIFT_IDX },  /* CapsLock */

  /* Teclas de funcion */
  [0x3b] = { '\0', '\0' },  /* F1 */
  [0x3c] = { '\0', '\0' },  /* F2 */
  [0x3d] = { '\0', '\0' },  /* F3 */
  [0x3e] = { '\0', '\0' },  /* F4 */
  [0x3f] = { '\0', '\0' },  /* F5 */
  [0x40] = { '\0', '\0' },  /* F6 */
  [0x41] = { '\0', '\0' },  /* F7 */
  [0x42] = { '\0', '\0' },  /* F8 */
  [0x43] = { '\0', '\0' },  /* F9 */
  [0x44] = { '\0', '\0' },  /* F10 */

  [0x45] = { '\0', '\0' },  /* NumLock */
  [0x46] = { '\0', '\0' },  /* ScrollLock */

  /* Teclado numerico */
  [0x47] = { '\0', '\0' },          /* Keypad-7/Home */
  [0x48] = { '\0', MOVE_UP_IDX },   /* Keypad-8/Up */
  [0x49] = { '\0', '\0' },          /* Keypad-9/PgUp */
  [0x4a] = { '\0', '\0' },          /* Keypad-- */
  [0x4b] = { '\0', MOVE_LEFT_IDX }, /* Keypad-4/Left */
  [0x4c] = { '\0', '\0' },          /* Keypad-5 */
  [0x4d] = { '\0', MOVE_RIGHT_IDX },/* Keypad-6/Right */
  [0x4e] = { '\0', '\0' },          /* Keypad-+ */
  [0x4f] = { '\0', '\0' },          /* Keypad-1/End */
  [0x50] = { '\0', MOVE_DOWN_IDX }, /* Keypad-2/Down */
  [0x51] = { '\0', '\0' },          /* Keypad-3/PgDn */
  [0x52] = { '\0', '\0' },          /* Keypad-0/Ins */
  [0x53] = { '\0', '\0' },          /* Keypad-./Del */
  [0x54] = { '\0', '\0' },          /* Alt-SysRq en teclado 84+ */
  [0x57] = { '\0', '\0' },          /* F11 en teclado 101+ */
  [0x58] = { '\0', '\0' }           /* F12 en teclado 101+ */
};

/* Funciones auxiliares para manejo del cursor */

static inline ca(*get_video_buffer(void))[VIDEO_COLS] {
  return (ca(*)[VIDEO_COLS])VIDEO;
}

static void fix_cursor_position(void) {
  int old_column = cursor.column;
  cursor.column = ((cursor.column % VIDEO_COLS) + VIDEO_COLS) % VIDEO_COLS;

  cursor.row += (old_column - cursor.column) / VIDEO_COLS;
  cursor.row = ((cursor.row % VIDEO_FILS) + VIDEO_FILS) % VIDEO_FILS;
}

static void put_character(char c) {
  ca(*screen)[VIDEO_COLS] = get_video_buffer();
  screen[cursor.row][cursor.column].c = c;
  screen[cursor.row][cursor.column].a = cursor.attr;
}

static void put_marker(void) {
  ca(*screen)[VIDEO_COLS] = get_video_buffer();
  cursor.under_marker = screen[cursor.row][cursor.column];
  screen[cursor.row][cursor.column].c = 0xDB;
  screen[cursor.row][cursor.column].a = cursor.attr | C_BLINK;
}

static void restore_marker(void) {
  ca(*screen)[VIDEO_COLS] = get_video_buffer();
  screen[cursor.row][cursor.column] = cursor.under_marker;
}

/* Handlers de scancodes especiales */

static void unrecognized_scancode(uint8_t scancode) {
  if (!(scancode & 0x80))
    print_hex(scancode, 2, 80/2 - 1, 50/2, C_FG_CYAN | C_FG_BLUE);
}

static void toggle_shift(unused uint8_t scancode) {
  is_shift_pressed = !is_shift_pressed;
}

static void move_down(unused uint8_t scancode) {
  restore_marker();
  cursor.row++;
  fix_cursor_position();
  put_marker();
}

static void move_up(unused uint8_t scancode) {
  restore_marker();
  cursor.row--;
  fix_cursor_position();
  put_marker();
}

static void move_left(unused uint8_t scancode) {
  restore_marker();
  cursor.column--;
  fix_cursor_position();
  put_marker();
}

static void move_right(unused uint8_t scancode) {
  restore_marker();
  cursor.column++;
  fix_cursor_position();
  put_marker();
}

static void new_line(unused uint8_t scancode) {
  restore_marker();
  cursor.column = 0;
  cursor.row++;
  fix_cursor_position();
  put_marker();
}

/* Funcion principal de procesamiento de scancodes */
void process_scancode(uint8_t scancode) {
  struct scancode_info info = scancode_defs[scancode];

  if (info.main_value == '\0') {
    special_scancodes[info.special_value](scancode);
    return;
  }

  put_character(is_shift_pressed ? info.special_value : info.main_value);

  cursor.column++;
  fix_cursor_position();
  put_marker();
}
