#include "led_module.h"

void LED_MODULE_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(LED_MODULE_CLK, ENABLE ); //≈‰÷√ ±÷”
	
	GPIO_InitStructure.GPIO_Pin = LED_MODULE_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(LED_MODULE_GPIO_PORT,&GPIO_InitStructure);

	LED_MODULE_OFF;
}

