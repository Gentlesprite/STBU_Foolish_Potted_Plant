#include "stm32f10x.h"
#include "Delay.h"
#include "led.h"
#include "DHT11.h"
#include "usart1.h"
#include "usart2.h"
#include "LDR.h"
#include "usart3.h"
#include "esp32_c3.h"
#include "LED.h"
#include "adc.h"
#include "OLED_I2C.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "timer1.h"
#include "led_module.h"
//#include "timer3.h"
volatile uint8_t Timer2_OLEDRefresh_Counter =0;//定时器2计时变量 3ms
volatile uint8_t Timer2_Sensor_Counter = 0;//传感器数据采集计时变量 200ms
volatile uint8_t FAN_Ctrl =0;//风扇控制标志位
volatile uint8_t MOTOR_Ctrl =1;//电机控制标志位
volatile uint8_t BEEP_Ctrl =1;//蜂鸣器控制标志位
volatile uint8_t Wifi_Rx_Counter =0;//数据接收轮询任务
volatile uint8_t LED_Ctrl =1;//数据接收轮询任务

//设定的默认报警阈值
u8 humidity_threshold = 80;//空气湿度默认阈值
u8 temp_threshold = 29;//温度默认阈值
int soil_threshold = 30;//土壤 湿度默认阈值
uint16_t light_threshold = 110; //灯光开启默认阈值
u16 light; //光照强度
//u16 led0pwmval=40; led pwm时开启
extern u8 USART3_RX_FLAG;//wifi接收标志位
char sendBuffer[512];
void send_normal_data_to_app(void);
void env_check(u8 temp,int soil,uint16_t co2);
typedef unsigned char u8;
char rxdata[100]="n";
u8 temperature;
u8 humidity;
uint16_t co2;
char display[16];
unsigned char setn=0;//记录设置键按下的次数
unsigned char temperature=0;
unsigned char humidity=0;
unsigned char setTempValue=35;        //温度设置值
unsigned int  setSoilMoisture=10;
unsigned int  soilMoisture=10;           //土壤湿度
unsigned char setLightValue=20;       //光照设置值

bool shuaxin  = 0;
bool shanshuo = 0;
bool sendFlag = 1;

const char* cmd = "\t\r\n↓↓↓支持的命令↓↓↓\r\n"
                  "[temp value] - 设置温度的报警阈值。\r\n"
//                  "[hum value] - 设置湿度的报警阈值。\r\n"
                  "[soil value] - 设置土壤湿度的报警阈值。\r\n"
                  "[light value] - 设置灯光开启阈值。\r\n"
                  "[data] - 获取当前环境的信息。\r\n"
                  "[FAN ON/OFF] - 排气扇启停。\r\n"
									"[BEEP ON/OFF] - 报警启停。\r\n"
                  "[MOTOR ON/OFF] - 水泵启停。";

void InitDisplay(void)   //初始化显示
{
	  unsigned char i=0;
	  for(i=0;i<4;i++)OLED_ShowCN(i*16,0,i+0,0);//显示中文：环境温度：
	  for(i=0;i<4;i++)OLED_ShowCN(i*16,2,i+4,0);//显示中文：环境湿度：
	  for(i=0;i<4;i++)OLED_ShowCN(i*16,4,i+8,0);//显示中文：土壤湿度：
		for(i=0;i<4;i++)OLED_ShowCN(i*16,6,i+21,0);//显示中文：光照强度：
	  OLED_ShowChar(64,0,':',2,0);
	  OLED_ShowChar(64,2,':',2,0);
    OLED_ShowChar(64,4,':',2,0);
		OLED_ShowChar(64,6,':',2,0);
		Delay_ms(1000);
		sprintf(sendBuffer,cmd);
		esp_32c3_send_data((u8 *)sendBuffer, 100);
}
void displayDHT11TempAndHumi(void)  //显示环境温湿度
{
//		DHT11_Read_Data(&temperature,&humidity);//显示模块不负责采集数据
	  if(temperature>=setTempValue && shanshuo)
		{
			  OLED_ShowChar(78,0,' ',2,0);
				OLED_ShowChar(86,0,' ',2,0);
		}
		else
		{
				OLED_ShowChar(78,0,temperature/10+'0',2,0);
				OLED_ShowChar(86,0,temperature%10+'0',2,0);
		}
		OLED_ShowCentigrade(94, 0);
		OLED_ShowChar(78,2,humidity/10+'0',2,0);
		OLED_ShowChar(86,2,humidity%10+'0',2,0);
		OLED_ShowChar(94,2,'%',2,0);
}
void displaySoilMoisture(void)//显示土壤湿度
{

//	   soilMoisture = 100-(Get_Adc_Average(ADC_Channel_8,10)*99/4096);
	   if(soilMoisture>99)soilMoisture=99;
		 if(soilMoisture<=setSoilMoisture && shanshuo)
		{
			  OLED_ShowChar(78,4,' ',2,0);
				OLED_ShowChar(86,4,' ',2,0);
		}
		else
		{
			 OLED_ShowChar(78,4,soilMoisture/10+'0',2,0);
			 OLED_ShowChar(86,4,soilMoisture%10+'0',2,0);
		}
  	 OLED_ShowChar(94,4,'%',2,0);
}

void displayCO2(void)    //显示二氧化碳浓度
{
    if(co2 > 999) co2 = 999;  //限制最大显示值
    OLED_ShowChar(78,6,(co2%1000)/100+'0',2,0); //百位
    OLED_ShowChar(86,6,(co2%100)/10+'0',2,0);  //十位
    OLED_ShowChar(94,6,co2%10+'0',2,0);       //个位
    OLED_ShowStr(102,6,"ppm",2,0);          //单位
}

void displayLight(void) {
    if (light > 999) light = 999;  // 限制最大值为999
    
    // 显示百位
    OLED_ShowChar(78, 6, (light % 1000) / 100 + '0', 2, 0);  // 百位
    
    // 显示十位
    OLED_ShowChar(86, 6, (light % 100) / 10 + '0', 2, 0);    // 十位
    
    // 显示个位
    OLED_ShowChar(94, 6, light % 10 + '0', 2, 0);            // 个位
    
    // 显示单位 "lux"
    OLED_ShowStr(102, 6, "Lux", 2, 0);                       // 单位
}
// 解析接收到的命令
void ParseCommand(char* cmd) {
    char* token;
    char* rest = cmd;
    
    // 获取第一个token（命令类型）
    token = strtok_r(rest, " ", &rest);
    
    if (token == NULL)return;

    if (strcmp(token, "temp") == 0) {
        // 温度阈值设置
        token = strtok_r(rest, " ", &rest);
        if (token != NULL) {
            temp_threshold = atoi(token);
						sprintf(sendBuffer, "设置温度阈值为:%s", token);
						esp_32c3_send_data((u8 *)sendBuffer, 50);
        }
    }
		else if (strcmp(token, "hum") == 0) {
        // 温度阈值设置
        token = strtok_r(rest, " ", &rest);
        if (token != NULL) {
            humidity_threshold = atoi(token);
						sprintf(sendBuffer, "设置湿度阈值为:%s", token);
						esp_32c3_send_data((u8 *)sendBuffer, 50);
        }
    }
		
    else if (strcmp(token, "soil") == 0) {
        // 土壤湿度阈值设置
        token = strtok_r(rest, " ", &rest);
        if (token != NULL) {
            soil_threshold = atoi(token);
						sprintf(sendBuffer, "设置土壤湿度阈值为:%s", token);
						esp_32c3_send_data((u8 *)sendBuffer, 50);
        }
    }
		
    else if (strcmp(token, "light") == 0) {
        // 灯光阈值设置
        token = strtok_r(rest, " ", &rest);
        if (token != NULL) {
            light_threshold = atoi(token);
						sprintf(sendBuffer, "设置灯光开启阈值为:%s", token);
						esp_32c3_send_data((u8 *)sendBuffer, 50);
        }
    }
		
		else if (strcmp(token, "data") == 0){
			sprintf(sendBuffer, "温度:%d℃ 湿度:%d%%RH 土壤湿度:%d%%RH 光照强度:%dLux", temperature, humidity, soilMoisture,light);
			esp_32c3_send_data((u8 *)sendBuffer, 50);
		}
		
		else if (strcmp(token, "FANON") == 0){
			FAN_ON();

			sprintf(sendBuffer, "排气扇已开启");
			esp_32c3_send_data((u8 *)sendBuffer, 50);
		}
		
		else if (strcmp(token, "FANOFF") == 0){
			FAN_OFF();

			sprintf(sendBuffer, "排气扇已关闭");
			esp_32c3_send_data((u8 *)sendBuffer, 50);
		}

		else if (strcmp(token, "MOTORON") == 0){
			RELAY_ON();
			if(MOTOR_Ctrl <1)
			{
				MOTOR_Ctrl +=1;
			}
			sprintf(sendBuffer, "水泵已开启");
			esp_32c3_send_data((u8 *)sendBuffer, 50);
		}
		
		else if (strcmp(token, "MOTOROFF") == 0){
			RELAY_OFF();
			if(MOTOR_Ctrl >=1)
			{
				MOTOR_Ctrl -=1;
			}
			sprintf(sendBuffer, "水泵已关闭");
			esp_32c3_send_data((u8 *)sendBuffer, 50);
		}
		
		else if (strcmp(token, "LEDON") == 0){
			LEDM_ON();
//			if(LED_Ctrl <1)
//			{
//				LED_Ctrl +=1;
//			}
			sprintf(sendBuffer, "灯光已打开");
			esp_32c3_send_data((u8 *)sendBuffer, 50);
		}
		else if (strcmp(token, "LEDOFF") == 0){
			LEDM_OFF();
//			if(LED_Ctrl >=1)
//			{
//				LED_Ctrl -=1;
//			}
			sprintf(sendBuffer, "灯光已关闭");
			esp_32c3_send_data((u8 *)sendBuffer, 50);
		}
		else if (strcmp(token, "BEEPON") == 0){
			BUZZER_ON();
			if(BEEP_Ctrl <1)
			{
				BEEP_Ctrl +=1;
			}
			sprintf(sendBuffer, "报警已开启");
			esp_32c3_send_data((u8 *)sendBuffer, 50);
		}
		else if (strcmp(token, "BEEPOFF") == 0){
			BUZZER_OFF();
			if(BEEP_Ctrl >=1)
			{
				BEEP_Ctrl -=1;
			}
			sprintf(sendBuffer, "报警已关闭");
			esp_32c3_send_data((u8 *)sendBuffer, 50);
		}
    else {
			sprintf(sendBuffer,cmd);
			esp_32c3_send_data((u8 *)sendBuffer, 100);
			
    }
}
void Display_Task(void)//3msOLED刷新间隔
{
	if(Timer2_OLEDRefresh_Counter >= 3)
	{
			Timer2_OLEDRefresh_Counter =0;//清除计时
			
			displayDHT11TempAndHumi();
			displaySoilMoisture();
			displayLight();
	}
}
void Sensor_Task(void)//200ms数据采集间隔
{
	if(Timer2_Sensor_Counter >= 200)
	{	
		Timer2_Sensor_Counter =1;
		DHT11_Read_Data(&temperature,&humidity);//采集温湿度
		CO2GetData(&co2);//采集CO2浓度
		light = LDR_LuxData();
		if(light>999)light=999;  //溢出限制
		soilMoisture = 100-(Get_Adc_Average(ADC_Channel_8,10)*99/4096);//采集土壤湿度
	  if(soilMoisture>99)soilMoisture=99;  //溢出限制
	}
}
void WiFi_Rx_Task(void)
{
	if(Wifi_Rx_Counter >= 50)
	{
		Wifi_Rx_Counter = 1;
		if(USART3_RX_FLAG)
			{
				// 确保添加终止符不会越界
				if(USART3_RX_STA < sizeof(USART3_RX_BUF))
					{
						USART3_RX_BUF[USART3_RX_STA] = '\0';
					} 
				else
					{
						USART3_RX_BUF[sizeof(USART3_RX_BUF)-1] = '\0';
					}		
				ParseCommand((char*)USART3_RX_BUF);
				LED_Hint(200);
				memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));// 清空接收缓冲区
				USART3_RX_STA = 0;
				USART3_RX_FLAG = 0;
			}
	}
}
int main(void)
{
	unsigned char i=0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	DelayInit();
	LED_Init();
	I2C_Configuration();     //IIC初始化
	OLED_Init();             //OLED液晶初始化
	LDR_Init();
	Adc_Init();
	OLED_CLS();              //清屏
	OLED_ShowStr(0, 3, "          ...", 2,0);//显示加载中
	for(i=0;i<5;i++)OLED_ShowCN(i*16,3,i+16,0);//显示中文：网络连接中
	usart1_init(115200);
	usart3_init(115200);
	tim1_init();
	TIM2_Init();
	LED_MODULE_Init();
	//TIM3_PWM_Init(100-1,0);	led pwm时开启
	LED_Hint(100);
	esp_32c3_init();
	LED_Hint(100);
	esp_32c3_send_cmd("AT+UARTTXDIS=1", "OK", 200);  // 禁用数据转发
	esp_32c3_send_cmd("AT+CIPMODE=0", "OK", 200);    // 退出透传模式
	esp_32c3_send_cmd("AT+CWQAP","0K",200);
	LED_Hint(100);
	esp_32c3_quit_init();
	LED_Hint(100);
	esp_32c3_start_init();
	LED_Hint(100);
	DHT11_Init();
	OLED_CLS();              //清屏
	InitDisplay();
	LED_Hint(100);
	while (1)
	{
			Sensor_Task();//传感器数据采集任务
			Display_Task();//OLED显示任务
			WiFi_Rx_Task();//控制命令接收
		}
}
