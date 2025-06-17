#ifndef __LED_MODULE_H
#define	__LED_MODULE_H
#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	大功率LED模块h文件                   
 * 版本			: V1.0
 * 日期			: 2024.9.23
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码							
 * BILIBILI	:	辰哥单片机设计
 * CSDN			:	辰哥单片机设计
 * 作者			:	辰哥

**********************BEGIN***********************/


/***************根据自己需求更改****************/
// 水泵模块 GPIO宏定义

#define	LED_MODULE_CLK							RCC_APB2Periph_GPIOB

#define LED_MODULE_GPIO_PIN 				GPIO_Pin_1

#define LED_MODULE_GPIO_PORT 				GPIOB

#define LED_MODULE_ON 							GPIO_SetBits(LED_MODULE_GPIO_PORT,LED_MODULE_GPIO_PIN)
#define LED_MODULE_OFF 							GPIO_ResetBits(LED_MODULE_GPIO_PORT,LED_MODULE_GPIO_PIN)

/*********************END**********************/

void LED_MODULE_Init(void);

#endif



