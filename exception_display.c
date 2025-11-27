
#include "exception_display.h"
#include "screen.h"
#include "colors.h"

/* Constantes para posicionamiento de informacion en pantalla */
#define GREGS_NAMES_START_COL 21
#define EFLAGS_NAME_START_COL 21
#define CREGS_NAMES_START_COL 46
#define GREGS_START_COL       25
#define EFLAGS_START_COL      28
#define CREGS_START_COL       50
#define STACK_START_COL       40

/* Mensajes descriptivos de excepciones */
const char* exception_messages[] = {
    "Divide Error #DE [0]",
    "Debug #DB [1]",
    "NMI Interrupt [2]",
    "Breakpoint #BP [3]",
    "Overflow #OF [4]",
    "BOUND Range Exceeded #BR [5]",
    "Invalid Opcode #UD [6]",
    "Device Not Available #NM [7]",
    "Double Fault (#DF) [8]",
    "Coprocessor Segment Overrun [9]",
    "Invalid TSS (#TS) [10]",
    "Segment Not Present (#NP) [11]",
    "Stack Fault (#SS) [12]",
    "General Protection (#GP) [13]",
    "Page-Fault (#PF) [14]",
    "RESERVED [15]",
    "Floating-Point Error (#MF) [16]",
    "Alignment Check (#AC) [17]",
    "Machine-Check (#MC) [18]",
    "SIMD Floating-Point (#XM) [19]",
    "Virtualization (#VE) [20]",
    "Control Protection (#CP) [21]"
};

/* Dibuja la plantilla de visualizacion de excepciones */
static void draw_exception_template(void) {
  screen_draw_box(1, 19, 40, 40, 0x0, 0x0);

  /* Nombres de registros generales */
  print("eax", GREGS_NAMES_START_COL, 6, 0x0F);
  print("ebx", GREGS_NAMES_START_COL, 8, 0x0F);
  print("ecx", GREGS_NAMES_START_COL, 10, 0x0F);
  print("edx", GREGS_NAMES_START_COL, 12, 0x0F);
  print("esi", GREGS_NAMES_START_COL, 14, 0x0F);
  print("edi", GREGS_NAMES_START_COL, 16, 0x0F);
  print("ebp", GREGS_NAMES_START_COL, 18, 0x0F);
  print("esp", GREGS_NAMES_START_COL, 20, 0x0F);
  print("eip", GREGS_NAMES_START_COL, 22, 0x0F);

  /* Nombres de registros de segmento */
  print(" cs", GREGS_NAMES_START_COL, 24, 0x0F);
  print(" ds", GREGS_NAMES_START_COL, 26, 0x0F);
  print(" es", GREGS_NAMES_START_COL, 28, 0x0F);
  print(" fs", GREGS_NAMES_START_COL, 30, 0x0F);
  print(" gs", GREGS_NAMES_START_COL, 32, 0x0F);
  print(" ss", GREGS_NAMES_START_COL, 34, 0x0F);
  print("eflags", EFLAGS_NAME_START_COL, 37, 0x0F);

  /* Nombres de registros de control */
  print("cr0", CREGS_NAMES_START_COL, 7, 0x0F);
  print("cr2", CREGS_NAMES_START_COL, 9, 0x0F);
  print("cr3", CREGS_NAMES_START_COL, 11, 0x0F);
  print("cr4", CREGS_NAMES_START_COL, 13, 0x0F);
  print("err", CREGS_NAMES_START_COL, 15, 0x0F);

  print("stack", STACK_START_COL, 19, 0x0F);

  /* Inicializar valores en 0 */
  print_hex(0, 8, GREGS_START_COL, 6, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 8, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 10, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 12, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 14, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 16, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 18, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 20, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 22, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 24, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 26, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 28, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 30, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 32, 0x0A);
  print_hex(0, 8, GREGS_START_COL, 34, 0x0A);
  print_hex(0, 8, EFLAGS_START_COL, 37, 0x0A);

  print_hex(0, 8, CREGS_START_COL, 7, 0x0A);
  print_hex(0, 8, CREGS_START_COL, 9, 0x0A);
  print_hex(0, 8, CREGS_START_COL, 11, 0x0A);
  print_hex(0, 8, CREGS_START_COL, 13, 0x0A);
  print_hex(0, 8, CREGS_START_COL, 15, 0x0A);
}

/* Imprime los valores actuales de los registros */
static void display_register_values(general_regs* gregs,
                                     segment_regs* sregs,
                                     control_regs* cregs,
                                     uint32_t eip,
                                     uint32_t eflags,
                                     uint32_t error_code) {
  /* Registros generales */
  print_hex(gregs->eax, 8, GREGS_START_COL, 6, 0x0A);
  print_hex(gregs->ebx, 8, GREGS_START_COL, 8, 0x0A);
  print_hex(gregs->ecx, 8, GREGS_START_COL, 10, 0x0A);
  print_hex(gregs->edx, 8, GREGS_START_COL, 12, 0x0A);
  print_hex(gregs->esi, 8, GREGS_START_COL, 14, 0x0A);
  print_hex(gregs->edi, 8, GREGS_START_COL, 16, 0x0A);
  print_hex(gregs->ebp, 8, GREGS_START_COL, 18, 0x0A);
  print_hex(gregs->esp, 8, GREGS_START_COL, 20, 0x0A);
  print_hex(eip, 8, GREGS_START_COL, 22, 0x0A);

  /* Registros de segmento */
  print_hex(sregs->cs, 8, GREGS_START_COL, 24, 0x0A);
  print_hex(sregs->ds, 8, GREGS_START_COL, 26, 0x0A);
  print_hex(sregs->es, 8, GREGS_START_COL, 28, 0x0A);
  print_hex(sregs->fs, 8, GREGS_START_COL, 30, 0x0A);
  print_hex(sregs->gs, 8, GREGS_START_COL, 32, 0x0A);
  print_hex(sregs->ss, 8, GREGS_START_COL, 34, 0x0A);
  print_hex(eflags, 8, EFLAGS_START_COL, 37, 0x0A);

  /* Registros de control */
  print_hex(cregs->cr0, 8, CREGS_START_COL, 7, 0x0A);
  print_hex(cregs->cr2, 8, CREGS_START_COL, 9, 0x0A);
  print_hex(cregs->cr3, 8, CREGS_START_COL, 11, 0x0A);
  print_hex(cregs->cr4, 8, CREGS_START_COL, 13, 0x0A);
  print_hex(error_code, 8, CREGS_START_COL, 15, 0x0A);
}

void display_kernel_exception(control_regs cregs,
                               segment_regs sregs,
                               general_regs gregs,
                               kernel_error_frame frame) {
  draw_exception_template();
  print(exception_messages[frame.exception_number], 20, 2, 0x0A);

  sregs.cs = frame.cs;
  display_register_values(&gregs, &sregs, &cregs, frame.eip, frame.eflags, frame.error_code);

  /* Mostrar contenido del stack */
  print_hex(*((uint32_t*)gregs.esp), 8, STACK_START_COL, 21, C_BG_BLACK | C_FG_LIGHT_GREEN);
  print_hex(*((uint32_t*)gregs.esp + 1), 8, STACK_START_COL, 23, C_BG_BLACK | C_FG_LIGHT_GREEN);
  print_hex(*((uint32_t*)gregs.esp + 2), 8, STACK_START_COL, 25, C_BG_BLACK | C_FG_LIGHT_GREEN);
}
