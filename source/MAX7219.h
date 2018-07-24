#ifndef MAX7219_
#define MAX7219_

#include "stm32f0xx.h"

// Output max7219  PA1= CS  PA2= CLK PA3= DIN
#define CS  1
#define CLK 2
#define DIN 3
#define Load_ON 1
#define Load_OFF 0

#define MAX7219_NoOperation						0xA0
#define MAX7219_DecodeMode						0x09
#define MAX7219_Intensity							0x0A
#define MAX7219_ScanLimit							0x0B
#define MAX7219_ShutDown							0x0C
#define MAX7219_DisplayTest						0x0F

extern unsigned char masiv_max7219[4];

void MAX7219_Init(void);
void MAX7219_DrawDigit1 (unsigned char Digit);
void MAX7219_DrawDigit2 (unsigned char Digit,unsigned char t);
void MAX7219_DrawDigit3 (unsigned char Digit);
void MAX7219_DrawDigit4 (unsigned char Digit);
void MAX7219_Indic (unsigned char *,unsigned char tochka);
void MAX7219_DisplayTestVoid (unsigned char Enable);
void MAX7219_DisplayClear(void);
void MAX7219_SetIntensity (unsigned char Intensity);
void MAX7219_Power (unsigned char PowerState);
void MAX7219_WriteReg (unsigned char Reg, unsigned char Data,unsigned char Load);



#endif

