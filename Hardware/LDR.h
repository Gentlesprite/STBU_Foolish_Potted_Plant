#ifndef __LDR_H
#define	__LDR_H
#include "stm32f10x.h"
#include "adcx.h"
#include "delay.h"
#include "math.h"

#define LDR_READ_TIMES	10  //光照传感器ADC循环读取次数

/***************根据自己需求更改****************/
// LDR GPIO宏定义
#define		LDR_GPIO_CLK							RCC_APB2Periph_GPIOC
#define 	LDR_GPIO_PORT							GPIOC
#define 	LDR_GPIO_PIN							GPIO_Pin_1
/*********************END**********************/


// ADC 通道宏定义
#define   ADC_CHANNEL               ADC_Channel_11

//#define    ADC_IRQ                       ADC3_IRQn
//#define    ADC_IRQHandler                ADC3_IRQHandler

void LDR_Init(void);
uint16_t LDR_Average_Data(void);
uint16_t LDR_LuxData();

#endif /* __ADC_H */

