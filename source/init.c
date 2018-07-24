#include "init.h"

void init_clock_mcu()
{
	 // HSE
	  RCC->CR |= RCC_CR_HSEON;
		 while(!(RCC->CR & RCC_CR_HSERDY)) {};
		
		FLASH->ACR |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
		
		// PLL
		RCC->CR &= ~RCC_CR_PLLON;  // off pll
		while((RCC->CR & RCC_CR_PLLRDY) != 0) {};
		RCC->CFGR |= RCC_CFGR_PLLMUL6;
	  RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV; 
		
		RCC->CR |= RCC_CR_PLLON; 
    while((RCC->CR & RCC_CR_PLLRDY) == 0){};
			
		RCC->CFGR |= RCC_CFGR_SW_PLL; 
		while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {};
		
	// MCO test clock
	  // no in 030f4 ....
}

void init_i2c(void)
{
	
	RCC->CFGR3 |= RCC_CFGR3_I2C1SW_SYSCLK;  // takt 48 Mhz
	RCC->AHBENR |=RCC_AHBENR_GPIOAEN;
	RCC->APB1ENR |=RCC_APB1ENR_I2C1EN;
	
	// PA9- SCL PA10- SDA  AF4
	GPIO_INIT(GPIOA, 9, GPIO_MODE_ALTFUNC, GPIO_OUTTYPE_OD, GPIO_PULL_NONE, GPIO_SPEED_HI, 4);
  GPIO_INIT(GPIOA, 10, GPIO_MODE_ALTFUNC, GPIO_OUTTYPE_OD, GPIO_PULL_NONE, GPIO_SPEED_HI, 4);
  
	
		/* (1) Timing register value is computed with the AN4235 xls file,
	fast Mode @400kHz with I2CCLK = 48MHz, rise time = 140ns,
	fall time = 40ns */
	/* (2) Periph enable */
	/* (3) Slave address = 0x5A, write transfer, 1 byte to transmit, autoend */
	I2C1->TIMINGR = (uint32_t)0x10805E89; /* (1) */
	I2C1->CR1 = I2C_CR1_PE; /* (2) */
	I2C1->CR2 = /*I2C_CR2_AUTOEND | */(2 << 16) | (0x68 <<1) ; /* (3) */
	 
}

void init(void)
{
	unsigned int test=0;
	//init clock HSE-PLL- 48 Mzh + CSS 
	init_clock_mcu();
	init_i2c();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; 
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // taktiro 48 Mzh=1/48= 21 nC
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	
	GPIO_INIT(GPIOA, 4, GPIO_MODE_OUTPUT, GPIO_OUTTYPE_PP,
          	GPIO_PULL_NONE, GPIO_SPEED_HI, GPIO_ALTFUNC_NONE);
	// tim1  500 mS
	// 1/48 Mhz= 21 ns  arr=63999  pres=374
	TIM1->ARR=63999;
	TIM1->PSC=374;
	TIM1->DIER |= TIM_DIER_UIE;
	
	//NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);  // on in zadache
	NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn,0);
	
	
	TIM1->CR1 |= TIM_CR1_CEN;
	
	// TIM3 Enkoder
	// PA7,PA6-> AF1  input nopullup(enkoder res 10 kOM) alternativ
	
		GPIO_INIT(GPIOA, 6, GPIO_MODE_ALTFUNC , GPIO_OUTTYPE_NONE, GPIO_PULL_NONE, GPIO_SPEED_HI, 1);
		GPIO_INIT(GPIOA, 7, GPIO_MODE_ALTFUNC, GPIO_OUTTYPE_NONE, GPIO_PULL_NONE, GPIO_SPEED_HI, 1);

    TIM3->CNT = 0 ;
		TIM3->ARR = 120 ;   //
		
    TIM3->CCMR1|= TIM_CCMR1_CC1S_0|TIM_CCMR1_CC2S_0 ; // IC2 is mapped on TI2
																											// IC1 is mapped on TI1
	  TIM3->CCMR1 |= TIM_CCMR1_IC1F | TIM_CCMR1_IC2F;   // filtr
	  TIM3->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1 ;  // SMS=011
	  
		TIM3->CR1 |= TIM_CR1_CEN;
		
		// Output max7219  PA1= CS  PA2= CLK PA3= DIN
			GPIO_INIT(GPIOA, 1, GPIO_MODE_OUTPUT, GPIO_OUTTYPE_PP,
          	GPIO_PULL_NONE, GPIO_SPEED_HI, GPIO_ALTFUNC_NONE);
	    GPIO_INIT(GPIOA, 2, GPIO_MODE_OUTPUT, GPIO_OUTTYPE_PP,
          	GPIO_PULL_NONE, GPIO_SPEED_HI, GPIO_ALTFUNC_NONE);
			GPIO_INIT(GPIOA, 3, GPIO_MODE_OUTPUT, GPIO_OUTTYPE_PP,
          	GPIO_PULL_NONE, GPIO_SPEED_HI, GPIO_ALTFUNC_NONE);
						
		// exti0 interput in fail
		//  PA0 input pullup
			GPIO_INIT(GPIOA, 0, GPIO_MODE_INPUT, GPIO_OUTTYPE_NONE,
          	GPIO_PULL_UP, GPIO_SPEED_HI, GPIO_ALTFUNC_NONE);
		EXTI->IMR = 0x0001; // EXTI0
		EXTI->FTSR = 0x0001; // fail
		NVIC_SetPriority(EXTI0_1_IRQn,0);
}



