#ifndef DELAY_
#define DELAY_

//#include "stm32f0xx.h"

#define CPU_CLOCK		48000000
#define K_Const			4800

void delay_ms(volatile unsigned long nTime);
void delay_us(volatile unsigned long nTime);

#endif

