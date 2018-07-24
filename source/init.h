#ifndef INIT_
#define INIT_

#include "stm32f0xx.h"

// Режим работы
#define GPIO_MODE_INPUT      0UL
#define GPIO_MODE_OUTPUT     1UL
#define GPIO_MODE_ALTFUNC    2UL
#define GPIO_MODE_ANALOG     3UL

// Тип выхода
#define GPIO_OUTTYPE_NONE    0UL   // заглушка
#define GPIO_OUTTYPE_PP      0UL
#define GPIO_OUTTYPE_OD      1UL

// Подтяжка
#define GPIO_PULL_NONE       0UL
#define GPIO_PULL_UP         1UL
#define GPIO_PULL_DOWN       2UL

// Скорость
#define GPIO_SPEED_LOW       0UL
#define GPIO_SPEED_MED       1UL
#define GPIO_SPEED_HI        3UL

// Альтернативные функции
#define GPIO_ALTFUNC_NONE    0UL   // заглушка

// Макрос инициализации
#define GPIO_INIT(GPIO_PORT,GPIO_PIN,GPIO_MODE,GPIO_OUTTYPE,GPIO_PULL,GPIO_SPEED,GPIO_ALTFUNC)\
{\
        GPIO_PORT->MODER = (GPIO_PORT->MODER & ~(3UL << (GPIO_PIN * 2UL)))\
        | (GPIO_MODE << (GPIO_PIN * 2UL));\
        GPIO_PORT->OTYPER = (GPIO_PORT->OTYPER & ~(1UL << (GPIO_PIN)))\
        | (GPIO_OUTTYPE << (GPIO_PIN));\
        GPIO_PORT->OSPEEDR = (GPIO_PORT->OSPEEDR & ~(3UL << (GPIO_PIN * 2UL)))\
        | (GPIO_SPEED << (GPIO_PIN * 2UL));\
        GPIO_PORT->PUPDR = (GPIO_PORT->PUPDR & ~(3UL << (GPIO_PIN * 2UL)))\
        | (GPIO_PULL << (GPIO_PIN * 2UL));\
        GPIO_PORT->AFR[GPIO_PIN / 8] = (GPIO_PORT->AFR[GPIO_PIN / 8] & ~(15UL << ((GPIO_PIN & 0x7) * 4)))\
        | ((GPIO_ALTFUNC) << (GPIO_PIN & 0x7) * 4);\
}

// Макросы для работы с ножкой порта
#define GPIO_SET(GPIO_PORT,GPIO_PIN) (GPIO_PORT->BSRR = (1 << GPIO_PIN))
#define GPIO_RESET(GPIO_PORT,GPIO_PIN) (GPIO_PORT->BRR = (1 << GPIO_PIN))
#define GPIO_TOGGLE(GPIO_PORT,GPIO_PIN) (GPIO_PORT->BSRR = (1 << (GPIO_PIN+ ((GPIO_PORT->ODR & (1 << GPIO_PIN))?16:0)
#define GPIO_ISSET(GPIO_PORT,GPIO_PIN) (GPIO_PORT->IDR & (1 << GPIO_PIN))
#define GPIO_ISRESET(GPIO_PORT,GPIO_PIN) (!(GPIO_PORT->IDR & (1 << GPIO_PIN)))

// Макросы для работы с портом
#define GPIOS_SET(GPIO_PORT,GPIO_MASK) (GPIO_PORT->BSRR = GPIO_MASK)
#define GPIOS_RESET(GPIO_PORT,GPIO_MASK) (GPIO_PORT->BRR = GPIO_MASK)
#define GPIOS_TOGGLE(GPIO_PORT,GPIO_MASK) (GPIO_PORT->ODR ^= GPIO_MASK)

// Примеры инициализации
// GPIO_INIT(GPIOA, 0, GPIO_MODE_OUTPUT, GPIO_OUTTYPE_PP, GPIO_PULL_NONE, GPIO_SPEED_HI, GPIO_ALTFUNC_NONE);
// GPIO_INIT(GPIOA, 1, GPIO_MODE_INPUT, GPIO_OUTTYPE_NONE, GPIO_PULL_UP, GPIO_SPEED_HI, GPIO_ALTFUNC_NONE);



extern void init_(void);
#endif

