
#ifndef __CPU_STATE_H__
#define __CPU_STATE_H__

#include "types.h"

/* Estructuras para representar el estado del CPU */

/* Registros de proposito general */
typedef struct general_regs_t {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
} general_regs;

/* Registros de segmento */
typedef struct segment_regs_t {
  uint32_t cs;
  uint32_t ds;
  uint32_t es;
  uint32_t fs;
  uint32_t gs;
  uint32_t ss;
} segment_regs;

/* Registros de control */
typedef struct control_regs_t {
  uint32_t cr0;
  uint32_t cr2;
  uint32_t cr3;
  uint32_t cr4;
} control_regs;

/* Frame de error para excepciones de usuario */
typedef struct error_frame_t {
  uint32_t exception_number;
  uint32_t error_code;
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
  uint32_t esp;
  uint32_t ss;
} error_frame;

/* Frame de error para excepciones del kernel */
typedef struct kernel_error_frame_t {
  uint32_t exception_number;
  uint32_t error_code;
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
} kernel_error_frame;

#endif // __CPU_STATE_H__
