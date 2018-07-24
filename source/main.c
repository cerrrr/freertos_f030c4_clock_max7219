
/* Standard includes. */
#include "string.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "queue.h"
#include "init.h"
#include "delays.h"
#include "MAX7219.h"

// prototip
void init(void);
void vTaskMain(void *pvParametrs);
void vTaskDS3231(void *pvParametrs);
void vAutoReloadTimerFunction(TimerHandle_t);


/*** descript RTOS ***/
TaskHandle_t xTask_main_Handle,xTask_setup_ds3231;
TimerHandle_t xTimer_mig;
SemaphoreHandle_t xSemaforSetup;
TimerHandle_t xTimer;
/*-----------------------------------------------------------*/
unsigned char mas_max7219[4];  // 0 and 1 -> clock  2 and 3-> minut
unsigned char tochka=0,temp_tochka=0;

// interput exti0 PA0
void EXTI0_1_IRQHandler(void)
{
	static BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	
	xHigherPriorityTaskWoken = pdFALSE;
	if(EXTI->PR & EXTI_PR_PIF0)
	{
		EXTI->PR |= EXTI_PR_PIF0;
		xSemaphoreGiveFromISR(xSemaforSetup, &xHigherPriorityTaskWoken);
	}
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

int main( void )
{
	
	
	char error_semafor,error_task1,error_task2,error_timer;
	BaseType_t xReturned1,xReturned2;
	
	
	init();
//	// test i2c
//	if ((I2C1->ISR & I2C_ISR_TXE) == I2C_ISR_TXE)
//	{
//	I2C1->TXDR = 0x01; /* Byte to send */
//	I2C1->CR2 |= I2C_CR2_START; /* Go */
//	}
//	while (!(I2C1->ISR & I2C_ISR_TXIS)) {__NOP(); };
//		I2C1->TXDR = 0x0F; /* Byte to send */
	  
	 // Timer Rtos
	xTimer=xTimerCreate
                   ( /* Just a text name, not used by the RTOS
                     kernel. */
                     "Timer",
                     /* The timer period in ticks, must be
                     greater than 0. */
                     500/portTICK_PERIOD_MS,
                     /* The timers will auto-reload themselves
                     when they expire. */
                     pdTRUE,
                     /* The ID is used to store a count of the
                     number of times the timer has expired, which
                     is initialised to 0. */
                     ( void * ) 0,
                     /* Each timer calls the same callback when
                     it expires. */
                     vAutoReloadTimerFunction
                   );
	if( xTimer == NULL )
         {
             /* The timer was not created. */
					   error_timer=1;
         }
         else
         {
             /* Start the timer.  No block time is specified, and
             even if one was it would be ignored because the RTOS
             scheduler has not yet been started. */
             if( xTimerStart( xTimer, 0 ) != pdPASS )
             {
                 /* The timer could not be set into the Active
                 state. */
							   error_timer=0;
             }
         }
	// semafor setup ext prerivanie button
		xSemaforSetup=xSemaphoreCreateBinary();
		if(xSemaforSetup==NULL)
		{ while(1) error_semafor=1; }
		
		
			// Task
	xReturned1=xTaskCreate( vTaskMain, 
		( const char * ) "TaskMain",
		configMINIMAL_STACK_SIZE,
		NULL, 
		2,
		&xTask_main_Handle );
		
		if( xReturned1 == pdPASS )
    {
        error_task1=0;
    }
	
	xReturned2=xTaskCreate( vTaskDS3231, ( const char * ) "TaskDS3231", 
		           configMINIMAL_STACK_SIZE, 
								NULL, 1, &xTask_setup_ds3231 );

	if( xReturned2 == pdPASS )
    {
        error_task2=0;
    }
	//xTP1.period=sizeof(tskTCB);
	vTaskStartScheduler();
		
  for( ;; );
	return 0;
}
/*-----------------------------------------------------------*/

//////////////////////////////////////////////////////////////////////////
// Task Main indic,read ds3231
void vTaskMain(void *pvParametrs)
{
	unsigned char hour=0,minute=0;
	MAX7219_Init();
	//MAX7219_WriteReg(0,0xff);
	for(;;)
	{
		//read ds3231
		  I2C1->CR2 = /*I2C_CR2_AUTOEND | */(1 << 16) | (0x68 <<1) ;  // D0 adress <- 1
			if ((I2C1->ISR & I2C_ISR_TXE) == I2C_ISR_TXE)
     	{
      	I2C1->TXDR = 0x01; /* Byte to send */
				I2C1->CR2 |= I2C_CR2_START; /* Go */
			}
			while (!(I2C1->ISR & I2C_ISR_TC)) {__NOP(); }; // end transmit
			// D1 adress <- 1
      I2C1->CR2 = /*I2C_CR2_AUTOEND | */ I2C_CR2_RD_WRN| (2 << 16) | (0x68 <<1) ; 
      I2C1->CR2 |= I2C_CR2_START; /* Go */
      while (!(I2C1->ISR & I2C_ISR_RXNE)) {__NOP(); };
			minute=I2C1->RXDR;
			while (!(I2C1->ISR & I2C_ISR_RXNE)) {__NOP(); };
			hour=I2C1->RXDR;
			while (!(I2C1->ISR & I2C_ISR_TC)) {__NOP(); }; // end recerve
			I2C1->CR2 |= I2C_CR2_STOP;
			
			// minut and hour
			mas_max7219[2]=(minute&0xf0)>>4;
			mas_max7219[3]=minute&0x0f;
			if(hour&0x20)
				mas_max7219[0]=2;
			else if(hour&0x10)
				mas_max7219[0]=1;
			else mas_max7219[0]=0;
			mas_max7219[1]=hour&0x0f;
		MAX7219_Indic(mas_max7219,tochka);
		vTaskDelay(200/portTICK_PERIOD_MS);
	}
	
}


//////////////////////////////////////////////////////////////////////////
// Task Setup input clock,write ds3231
void vTaskDS3231(void *pvParametrs)
{
	unsigned char button=0,temp=0;
	unsigned char clock_setup=0,minut_setup=0;
	unsigned char flag_mig=0;
	unsigned char clock=0,minut=0;
	
	NVIC_EnableIRQ(EXTI0_1_IRQn);
	for(;;)
	{
		if( xSemaforSetup != NULL )
    {
       xSemaphoreTake( xSemaforSetup, portMAX_DELAY );
			 NVIC_DisableIRQ(EXTI0_1_IRQn);
			 vTaskSuspend( xTask_main_Handle );  // off task min
       // first menu clock
			 // delay PA0 if pulldown
			 while(GPIO_ISRESET(GPIOA,0)) __NOP;  // delay
			 delay_ms(5);
			 MAX7219_DisplayClear();
			 // clock
			 mas_max7219[0]=10;
			 MAX7219_DrawDigit1(mas_max7219[0]); 
			 TIM3->CNT = 0 ;
		   TIM3->ARR = 48 ;   //
			 while(GPIO_ISSET(GPIOA,0)) 
			 {
				 clock_setup=TIM3->CNT/2;
				 if(clock_setup>23)
					 clock_setup=0;
				 temp=clock_setup/10;
				 mas_max7219[2]=temp;
				 if(temp)
				 {
						mas_max7219[3]=clock_setup-(temp*10);
					  flag_mig=1;
				 }
				 else
				 {
					  mas_max7219[3]=clock_setup;
					  if(flag_mig)
						{
							MAX7219_DisplayClear();
							flag_mig=0;
						}
				 }
				 MAX7219_DrawDigit1(mas_max7219[0]); 
				 if(mas_max7219[2])
					MAX7219_DrawDigit3(mas_max7219[2]);
				 MAX7219_DrawDigit4(mas_max7219[3]);
				 delay_ms(20);
			 }
			 clock=(mas_max7219[2]<<4)|mas_max7219[3];
			 
			 //  minut setup
			 while(GPIO_ISRESET(GPIOA,0)) __NOP;  // delay
			 delay_ms(5);
			 
			 mas_max7219[0]=11;
			 MAX7219_DrawDigit1(mas_max7219[0]); 
			 TIM3->CNT = 0 ;
		   TIM3->ARR = 120 ;   //
			 while(GPIO_ISSET(GPIOA,0)) 
			 {
				  clock_setup=TIM3->CNT/2;
				 if(clock_setup>59)
					 clock_setup=0;
				 temp=clock_setup/10;
				 mas_max7219[2]=temp;
				 if(temp)
				 {
						mas_max7219[3]=clock_setup-(temp*10);
					  flag_mig=1;
				 }
				 else
				 {
					  mas_max7219[3]=clock_setup;
					  if(flag_mig)
						{
							MAX7219_DisplayClear();
							flag_mig=0;
						}
				 }
				 MAX7219_DrawDigit1(mas_max7219[0]); 
				 if(mas_max7219[2])
					MAX7219_DrawDigit3(mas_max7219[2]);
				 MAX7219_DrawDigit4(mas_max7219[3]);
				 delay_ms(20);
				}
			  minut=(mas_max7219[2]<<4)|mas_max7219[3];
				
				while(GPIO_ISRESET(GPIOA,0)) __NOP;  // delay
			 delay_ms(20);
				
				MAX7219_DisplayClear();
				// write in ds3231 clock and minut
				I2C1->CR2 = I2C_CR2_AUTOEND | (3 << 16) | (0x68 <<1) ;  // D0 adress <- 1
			if ((I2C1->ISR & I2C_ISR_TXE) == I2C_ISR_TXE)
     	{
      	I2C1->TXDR = 0x01; /* Byte to send */
				I2C1->CR2 |= I2C_CR2_START; /* Go */
			}
			while(!(I2C1->ISR & I2C_ISR_TXIS)) {__NOP(); };
			I2C1->TXDR =minut;
			while(!(I2C1->ISR & I2C_ISR_TXIS)) {__NOP(); };
			I2C1->TXDR = clock;
			
			//while (!(I2C1->ISR & I2C_ISR_TC)) {__NOP(); }; // end transmit
				__NOP();
			EXTI->PR |= EXTI_PR_PIF0;
			NVIC_EnableIRQ(EXTI0_1_IRQn);
			
			vTaskResume( xTask_main_Handle );
		}
		//vTaskDelay(100/portTICK_PERIOD_MS);
	}
	
}



///////////////////////////////////////////////////////////////////////////
// Timer mig 0.5 sec 
//////////////////////////////////////////////////////////
// TIMER RTOS
void vAutoReloadTimerFunction(TimerHandle_t xTimer) {
	if(temp_tochka)
	{
		tochka=1;
		temp_tochka=0;
	}
	else
	{
		tochka=0;
		temp_tochka=1;
	}
	GPIOA->ODR ^= GPIO_ODR_4 ;
	
}


//---------------------------------------------------------------

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	added here, but the tick hook is called from an interrupt context, so
	code must not attempt to block, and only the interrupt safe FreeRTOS API
	functions can be used (those that end in FromISR()). */
}
/*-----------------------------------------------------------*/

#ifdef JUST_AN_EXAMPLE_ISR

void Dummy_IRQHandler(void)
{
long lHigherPriorityTaskWoken = pdFALSE;

	/* Clear the interrupt if necessary. */
	Dummy_ClearITPendingBit();

	/* This interrupt does nothing more than demonstrate how to synchronise a
	task with an interrupt.  A semaphore is used for this purpose.  Note
	lHigherPriorityTaskWoken is initialised to zero. Only FreeRTOS API functions
	that end in "FromISR" can be called from an ISR. */
	xSemaphoreGiveFromISR( xTestSemaphore, &lHigherPriorityTaskWoken );

	/* If there was a task that was blocked on the semaphore, and giving the
	semaphore caused the task to unblock, and the unblocked task has a priority
	higher than the current Running state task (the task that this interrupt
	interrupted), then lHigherPriorityTaskWoken will have been set to pdTRUE
	internally within xSemaphoreGiveFromISR().  Passing pdTRUE into the
	portEND_SWITCHING_ISR() macro will result in a context switch being pended to
	ensure this interrupt returns directly to the unblocked, higher priority,
	task.  Passing pdFALSE into portEND_SWITCHING_ISR() has no effect. */
	portEND_SWITCHING_ISR( lHigherPriorityTaskWoken );
}

#endif /* JUST_AN_EXAMPLE_ISR */




