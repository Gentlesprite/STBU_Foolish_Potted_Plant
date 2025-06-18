#include "stm32f10x_tim.h"
#include "timer1.h"
#include "led.h"
extern u8 temperature;
extern u8 humidity;
extern int soilMoisture;
extern u16 light;

extern u8 temp_threshold;
extern u8 humidity_threshold;
extern int soil_threshold;
extern uint16_t light_threshold;

void tim1_init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM1时钟（注意：TIM1在APB2总线上）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    
    // 定时器基础配置（500ms中断）
    TIM_TimeBaseStructure.TIM_Period = 4999;     // 自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = 7199;  // 预分频值（72MHz / 7200 = 10kHz）
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0; // 高级定时器特有参数
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    
    // 使能更新中断（注意：TIM1需额外使能主输出）
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE); // 高级定时器特有
    
    // NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn; // TIM1中断通道不同！
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动定时器
    TIM_Cmd(TIM1, ENABLE);
}

extern uint8_t FAN_Ctrl;//排气扇控制标志位
extern uint8_t MOTOR_Ctrl;//窗帘控制标志位
extern uint8_t LED_Ctrl;//排气扇控制标志位
extern uint8_t BEEP_Ctrl;//排气扇控制标志位
void TIM1_UP_IRQHandler(void) {
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
				if(temperature > temp_threshold)FAN_ON();
				else if(FAN_Ctrl == 0)FAN_OFF();
			
				if(humidity > humidity_threshold)RELAY_ON();
				else if(MOTOR_Ctrl == 0)RELAY_OFF();

				if(light < light_threshold)LEDM_ON(); 
				else if(LED_Ctrl == 0)LEDM_OFF();

				if(soilMoisture > soil_threshold)BUZZER_ON();
				else if(BEEP_Ctrl == 0)BUZZER_OFF();
			
    }
}

void TIM2_Init(void) {
    TIM_TimeBaseInitTypeDef timer;
    NVIC_InitTypeDef nvic;
    
    // 时钟使能
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
		 // 定时器配置（1ms 中断）
    timer.TIM_Prescaler = 7200 - 1;  // 72MHz / 7200 = 10kHz
    timer.TIM_Period = 10 - 1;       // 10kHz / 10 = 1kHz (1ms)
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &timer);
    
    // 中断优先级设置
    nvic.NVIC_IRQChannel = TIM2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;  // 抢占优先级1
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
    
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}


extern uint8_t Timer2_Sensor_Counter;//传感器 200ms
extern uint8_t Timer2_OLEDRefresh_Counter;//定时器2计时变量 3ms
extern uint16_t LED_Hint_Counter ;//LED提示灯0-600s
extern uint8_t Wifi_Rx_Counter ;//数据接收轮询任务
void TIM2_IRQHandler(void) {
		
    if (TIM_GetITStatus(TIM2, TIM_IT_Update)) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//				static uint8_t tick = 0;
//        Timer2_Sensor_Counter = (++tick) % 20; // 传感器 200ms 自动归零
			  ++Timer2_Sensor_Counter ;
				++Timer2_OLEDRefresh_Counter;//定时器2计时变量 3ms
				++Wifi_Rx_Counter;//WiFi数据接收50ms轮询
			if(--LED_Hint_Counter <= 0) LED2_OFF();//LED提示灯0-600s
    }
}
