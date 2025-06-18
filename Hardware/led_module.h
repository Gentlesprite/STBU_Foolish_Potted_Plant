#ifndef __LED_MODULE_H
#define	__LED_MODULE_H
#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"



#define	LED_MODULE_CLK							RCC_APB2Periph_GPIOB

#define LED_MODULE_GPIO_PIN 				GPIO_Pin_1

#define LED_MODULE_GPIO_PORT 				GPIOB

#define LED_MODULE_ON 							GPIO_SetBits(LED_MODULE_GPIO_PORT,LED_MODULE_GPIO_PIN)
#define LED_MODULE_OFF 							GPIO_ResetBits(LED_MODULE_GPIO_PORT,LED_MODULE_GPIO_PIN)

/*********************END**********************/

void LED_MODULE_Init(void);

#endif



