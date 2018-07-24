#include "MAX7219.h"
#include "delays.h"
#include "init.h"


unsigned char font_max7219[12][8]=
{
	{0x7C,0xC6,0xCE,0xDE,0xF6,0xE6,0x7C,0x0},  // 0
	{0x30,0x70,0x30,0x30,0x30,0x30,0x30,0x0},  // 1
	{0x78,0xCC,0xC,0x38,0x60,0xC0,0xFC,0x0},   // 2
	{0x78,0xCC,0xC,0x38,0xC,0xCC,0x78,0x0},    // 3
	{0x1C,0x3C,0x6C,0xCC,0xFE,0xC,0xC,0x0},    // 4
	{0xFC,0xC0,0xF8,0xC,0xC,0xCC,0x78,0x0},    // 5
	{0x38,0x60,0xC0,0xF8,0xCC,0xCC,0x78,0x0},  // 6
	{0xFC,0xC,0xC,0x18,0x30,0x30,0x30,0x0},    // 7
	{0x78,0xCC,0xCC,0x78,0xCC,0xCC,0x78,0x0},  // 8
	{0x78,0xCC,0xCC,0x7C,0xC,0x18,0x70,0x0},   // 9
	{0x66,0x66,0x66,0x3E,0x6,0x6,0x6,0x0},     // ?
	{0xC6,0xEE,0xFE,0xFE,0xD6,0xD6,0xC6,0x0}
};

///* You can manualy assign the value of chips in serialy connected here*/
//unsigned char ChipsInSerial=0;	//otherwise search procedure will be needed and loop must be closed

//unsigned char DigitsPerChip=8;

unsigned char temp_zero_0=0;  // memmory zero indic znakomesta 0

// 100 kHz = 10 mkS period
void MAX7219_WriteReg (unsigned char Reg, unsigned char Data,unsigned char Load)
{
	unsigned char cnt=0;
	unsigned char tmp=0;
	unsigned char reg_temp=0;
	
	  GPIO_SET(GPIOA,CS);  // CS->1
	  if(Load)
			GPIO_RESET(GPIOA,CS);
	  // 4 bit zero
	  GPIO_RESET(GPIOA,DIN);
		for (tmp=0;tmp<4;tmp++) 
			{ GPIO_SET(GPIOA,CLK); 
			  delay_us(1); 
			   GPIO_RESET(GPIOA,CLK); 
			   delay_us(1); }
		// 4 bit ADRES-Reg
		for (tmp=0;tmp<4;tmp++) 
			{ reg_temp=Reg&0x08;
				if(reg_temp)
					GPIO_SET(GPIOA,DIN); 
				else
					GPIO_RESET(GPIOA,DIN);				
			  GPIO_SET(GPIOA,CLK); 
				delay_us(1); 
				GPIO_RESET(GPIOA,CLK); 
				delay_us(1); 
			  Reg=Reg<<1;
			}
		// 8 bit infa
		for (tmp=0;tmp<8;tmp++) 
			{ reg_temp=Data&0x80; 
				if(reg_temp)
					GPIO_SET(GPIOA,DIN); 
				else
					GPIO_RESET(GPIOA,DIN);
			  GPIO_SET(GPIOA,CLK); 
				delay_us(1); 
				GPIO_RESET(GPIOA,CLK); 
				delay_us(1); 
			  Data=Data<<1;
			}	
		GPIO_SET(GPIOA,CS);
		GPIO_RESET(GPIOA,DIN);
}

void MAX7219_Init(void)
{
	
	

		MAX7219_DisplayTestVoid(0);
		MAX7219_WriteReg(MAX7219_DecodeMode, 0,Load_ON);
		MAX7219_WriteReg(MAX7219_Intensity, 0x07,Load_ON);
		MAX7219_WriteReg(MAX7219_ScanLimit, 0x07,Load_ON);
		MAX7219_WriteReg(MAX7219_ShutDown, 0x01,Load_ON);
    MAX7219_DisplayClear();
}

void MAX7219_DisplayClear(void)
{
	
	unsigned char temp=0,temp2=0;
		  // clear command
		for(temp=0;temp<4;temp++)
	    MAX7219_WriteReg(MAX7219_NoOperation, 0x00,Load_ON);
		//MAX7219_DisplayTestVoid(1);
	for(temp2=1;temp2<9;temp2++)
	{
		for(temp=0;temp<4;temp++)
		{
			MAX7219_WriteReg(temp2, 0x00,Load_ON);
		}
	}
	// clear command
		for(temp=0;temp<4;temp++)
	    MAX7219_WriteReg(MAX7219_NoOperation, 0x00,Load_ON);
}

// Digit 0=0 1=1 ... 9=9 10=× 11=Ì

void MAX7219_DrawDigit4 (unsigned char Digit)
{
	unsigned char temp=0,temp2=0,temp3=0;
	
	// 
	for(temp=1;temp<9;temp++)
	{
		MAX7219_WriteReg(temp,font_max7219[Digit][temp2],Load_ON);
		temp2++;
		// clear command
		for(temp3=0;temp3<4;temp3++)
	    MAX7219_WriteReg(MAX7219_NoOperation, 0x00,Load_OFF);
	}
}


void MAX7219_DrawDigit3 (unsigned char Digit)
{
	// masiv digit
	unsigned char temp=0,temp2=0,temp3=0;
	
	// 
	for(temp=1;temp<9;temp++)
	{
		MAX7219_WriteReg(MAX7219_NoOperation, 0x00,Load_OFF);
		MAX7219_WriteReg(MAX7219_NoOperation, 0x00,Load_OFF);
		MAX7219_WriteReg(temp,font_max7219[Digit][temp2],Load_OFF);
		temp2++;
	  MAX7219_WriteReg(MAX7219_NoOperation, 0x00,Load_ON);
	}
}

void MAX7219_DrawDigit2 (unsigned char Digit,unsigned char t)
{
	unsigned char temp=0,temp2=0,temp3=0;
	
	// 
	for(temp=1;temp<9;temp++)
	{
		MAX7219_WriteReg(MAX7219_NoOperation, 0x00,Load_OFF);
		if(temp==7&&t==1)
			MAX7219_WriteReg(temp,font_max7219[Digit][temp2]|0x01,Load_OFF);
		else
			MAX7219_WriteReg(temp,font_max7219[Digit][temp2],Load_OFF);
		temp2++;
		MAX7219_WriteReg(MAX7219_NoOperation, 0x00,Load_OFF);
	  MAX7219_WriteReg(MAX7219_NoOperation, 0x00,Load_ON);
	}
}

void MAX7219_DrawDigit1 (unsigned char Digit)
{
	unsigned char temp=0,temp2=0,temp3=0;
	
	// 
	for(temp=1;temp<9;temp++)
	{
		
		MAX7219_WriteReg(temp,font_max7219[Digit][temp2],Load_OFF);
		temp2++;
		// clear command
		for(temp3=0;temp3<2;temp3++)
	    MAX7219_WriteReg(MAX7219_NoOperation, 0x00,Load_OFF);
		MAX7219_WriteReg(MAX7219_NoOperation, 0x00,Load_ON);
	}
}

void MAX7219_Indic (unsigned char *pMas,unsigned char tochka)
{
	//-------------------------------------------------
	if(pMas[0])
	{ MAX7219_DrawDigit1(pMas[0]); temp_zero_0=0;}
	else if((!pMas[0])&(!temp_zero_0))
		{ MAX7219_DisplayClear(); temp_zero_0=1; }
	//-------------------------------------------------
	if(tochka)
		MAX7219_DrawDigit2(pMas[1],1);
	else
		MAX7219_DrawDigit2(pMas[1],0);
	//-------------------------------------------------
	MAX7219_DrawDigit3(pMas[2]);
	//-------------------------------------------------
	MAX7219_DrawDigit4(pMas[3]);
	
}


void MAX7219_DisplayTestVoid (unsigned char Enable)
{
	MAX7219_WriteReg(MAX7219_DisplayTest, Enable,Load_ON);
}

void MAX7219_SetIntensity (unsigned char Intensity)
{
	MAX7219_WriteReg(MAX7219_Intensity, Intensity&0x0F,Load_ON);
}

void MAX7219_Power (unsigned char PowerState)
{
	MAX7219_WriteReg(MAX7219_ShutDown, PowerState,Load_ON);
}


