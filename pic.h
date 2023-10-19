

#ifndef __PIC_H__
#define __PIC_H__

#include "stdint.h"

void pic_reset(void);
void pic_enable();
void pic_disable();

void pic_finish1(void);
void pic_finish2(void);

#endif //  __PIC_H__
