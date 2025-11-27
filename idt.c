
#include "idt.h"
#include "defines.h"
#include "i386.h"
#include "isr.h"
#include "cpu_state.h"
#include "exception_display.h"

/* Tabla de descriptores de interrupcion */
idt_entry_t idt[255] = {0};

/* Descriptor de la IDT para cargar con LIDT */
idt_descriptor_t IDT_DESC = {sizeof(idt) - 1, (uint32_t)&idt};

/* Macros para extraer bits de direcciones de 32 bits */
#define LOW_16_BITS(v) ((uint32_t)(v) & 0xFFFF)
#define HIGH_16_BITS(v) ((uint32_t)(v) >> 16 & 0xFFFF)

/* Niveles de privilegio */
#define DPL_KERNEL 0
#define DPL_USER   3

/* Crea una entrada en la IDT para interrupciones de nivel kernel (DPL=0) */
static void create_idt_entry_kernel(uint8_t index, uint32_t isr_address) {
  idt[index] = (idt_entry_t) {
    .offset_31_16 = HIGH_16_BITS(isr_address),
    .offset_15_0 = LOW_16_BITS(isr_address),
    .segsel = GDT_CODE_0_SEL,
    .type = INTERRUPT_GATE_TYPE,
    .dpl = DPL_KERNEL,
    .present = 1
  };
}

/* Crea una entrada en la IDT para interrupciones de nivel usuario (DPL=3) */
static void create_idt_entry_user(uint8_t index, uint32_t isr_address) {
  idt[index] = (idt_entry_t) {
    .offset_31_16 = HIGH_16_BITS(isr_address),
    .offset_15_0 = LOW_16_BITS(isr_address),
    .segsel = GDT_CODE_0_SEL,
    .type = INTERRUPT_GATE_TYPE,
    .dpl = DPL_USER,
    .present = 1
  };
}

/* Inicializa entradas de excepciones (0-20) */
static void init_exception_handlers(void) {
  create_idt_entry_kernel(0, (uint32_t)&_isr0);
  create_idt_entry_kernel(1, (uint32_t)&_isr1);
  create_idt_entry_kernel(2, (uint32_t)&_isr2);
  create_idt_entry_kernel(3, (uint32_t)&_isr3);
  create_idt_entry_kernel(4, (uint32_t)&_isr4);
  create_idt_entry_kernel(5, (uint32_t)&_isr5);
  create_idt_entry_kernel(6, (uint32_t)&_isr6);
  create_idt_entry_kernel(7, (uint32_t)&_isr7);
  create_idt_entry_kernel(8, (uint32_t)&_isr8);
  create_idt_entry_kernel(9, (uint32_t)&_isr9);
  create_idt_entry_kernel(10, (uint32_t)&_isr10);
  create_idt_entry_kernel(11, (uint32_t)&_isr11);
  create_idt_entry_kernel(12, (uint32_t)&_isr12);
  create_idt_entry_kernel(13, (uint32_t)&_isr13);
  create_idt_entry_kernel(14, (uint32_t)&_isr14);
  create_idt_entry_kernel(15, (uint32_t)&_isr15);
  create_idt_entry_kernel(16, (uint32_t)&_isr16);
  create_idt_entry_kernel(17, (uint32_t)&_isr17);
  create_idt_entry_kernel(18, (uint32_t)&_isr18);
  create_idt_entry_kernel(19, (uint32_t)&_isr19);
  create_idt_entry_kernel(20, (uint32_t)&_isr20);
}

/* Inicializa entradas de interrupciones de hardware */
static void init_hardware_interrupts(void) {
  create_idt_entry_kernel(32, (uint32_t)&_isr32);  /* CLOCK */
  create_idt_entry_kernel(33, (uint32_t)&_isr33);  /* TECLADO */
}

/* Inicializa entradas de syscalls de usuario */
static void init_user_syscalls(void) {
  create_idt_entry_user(88, (uint32_t)&_isr88);
  create_idt_entry_user(98, (uint32_t)&_isr98);
}

void idt_init(void) {
  init_exception_handlers();
  init_hardware_interrupts();
  init_user_syscalls();
}

/* Handler de excepciones del kernel */
void kernel_exception(control_regs cregs,
                      segment_regs sregs,
                      general_regs gregs,
                      kernel_error_frame frame) {
  display_kernel_exception(cregs, sregs, gregs, frame);
}
