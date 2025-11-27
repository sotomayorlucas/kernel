
#ifndef __EXCEPTION_DISPLAY_H__
#define __EXCEPTION_DISPLAY_H__

#include "cpu_state.h"
#include "types.h"

/* Mensajes de excepciones de CPU */
extern const char* exception_messages[];

/* Muestra informacion detallada de una excepcion del kernel */
void display_kernel_exception(control_regs cregs,
                               segment_regs sregs,
                               general_regs gregs,
                               kernel_error_frame frame);

#endif // __EXCEPTION_DISPLAY_H__
