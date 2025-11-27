
#include "pic.h"

/* Puertos de los PICs */
#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT    0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT    0xA1

/* Comandos del PIC */
#define PIC_EOI           0x20  /* End of Interrupt */
#define PIC_ICW1_INIT     0x11  /* Initialization Command Word 1 */
#define PIC_ICW4_8086     0x01  /* 8086 mode */

/* Offsets de interrupciones */
#define PIC1_OFFSET       32    /* IRQ 0-7 mapeadas a INT 32-39 */
#define PIC2_OFFSET       40    /* IRQ 8-15 mapeadas a INT 40-47 */

/* Configuracion de cascada */
#define PIC1_CASCADE_IRQ  0x04  /* IRQ2 es la cascada del PIC2 */
#define PIC2_CASCADE_ID   0x02  /* PIC2 esta conectado al IRQ2 del PIC1 */

/* Mascaras */
#define PIC_DISABLE_ALL   0xFF  /* Deshabilitar todas las IRQs */
#define PIC_ENABLE_ALL    0x00  /* Habilitar todas las IRQs */

/* Funcion inline para escribir a un puerto */
static __inline __attribute__((always_inline)) void outb(uint32_t port, uint8_t data) {
  __asm __volatile("outb %0,%w1" : : "a"(data), "d"(port));
}

/* Envia End of Interrupt al PIC1 */
void pic_finish1(void) {
  outb(PIC1_COMMAND_PORT, PIC_EOI);
}

/* Envia End of Interrupt al PIC1 y PIC2 */
void pic_finish2(void) {
  outb(PIC1_COMMAND_PORT, PIC_EOI);
  outb(PIC2_COMMAND_PORT, PIC_EOI);
}

/* Reinicia y configura los PICs */
void pic_reset(void) {
  /* Inicializacion del PIC1 */
  outb(PIC1_COMMAND_PORT, PIC_ICW1_INIT);
  outb(PIC1_DATA_PORT, PIC1_OFFSET);
  outb(PIC1_DATA_PORT, PIC1_CASCADE_IRQ);
  outb(PIC1_DATA_PORT, PIC_ICW4_8086);
  outb(PIC1_DATA_PORT, PIC_DISABLE_ALL);

  /* Inicializacion del PIC2 */
  outb(PIC2_COMMAND_PORT, PIC_ICW1_INIT);
  outb(PIC2_DATA_PORT, PIC2_OFFSET);
  outb(PIC2_DATA_PORT, PIC2_CASCADE_ID);
  outb(PIC2_DATA_PORT, PIC_ICW4_8086);
}

/* Habilita todas las interrupciones en ambos PICs */
void pic_enable(void) {
  outb(PIC1_DATA_PORT, PIC_ENABLE_ALL);
  outb(PIC2_DATA_PORT, PIC_ENABLE_ALL);
}

/* Deshabilita todas las interrupciones en ambos PICs */
void pic_disable(void) {
  outb(PIC1_DATA_PORT, PIC_DISABLE_ALL);
  outb(PIC2_DATA_PORT, PIC_DISABLE_ALL);
}
