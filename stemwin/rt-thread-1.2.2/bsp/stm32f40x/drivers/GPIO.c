/**********************************************************************************************************
*
*	模块名称 : LED驱动模块
*	文件名称 : led.c
*	版    本 : V1.0
*	说    明 :
*	修改记录 :
*		版本号  日期        作者           说明
*
*		v1.0    2012-11-19  wangkai        在官方基础上做调整,led当简易流水灯
*
*	Copyright (C), 2010-2011, UP MCU 工作室
*   淘宝店：   http://shop73275611.taobao.com
*   QQ交流群： 258043068
*
**********************************************************************************************************/
#include <rtthread.h>
#include <stm32f4xx.h>
#include "GPIO.h"

// led define






void rt_hw_GPIO_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOI , ENABLE);	

    //RCC_APB1PeriphClockCmd(ADM2587_1_EN,ENABLE);

//    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

//    GPIO_InitStructure.GPIO_Pin   = ADM2587_1_pin;
//    GPIO_Init(ADM2587_1_gpio, &GPIO_InitStructure);
//    /*开启rs485接收使能*/
//    RS485_1_RE;
//	
//	
//	  //RCC_APB1PeriphClockCmd(ADM2587_2_EN,ENABLE);

//    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

//    GPIO_InitStructure.GPIO_Pin   = ADM2587_2_pin;
//    GPIO_Init(ADM2587_2_gpio, &GPIO_InitStructure);
//    /*开启rs485接收使能*/
//    RS485_2_RE;
		
		
	  //RCC_APB1PeriphClockCmd(LED1_EN,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = LED1_pin;
    GPIO_Init(LED1_gpio, &GPIO_InitStructure);

    LED1_OFF;
		
		
		//RCC_APB1PeriphClockCmd(LED2_EN,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = LED2_pin;
    GPIO_Init(LED2_gpio, &GPIO_InitStructure);

    LED2_OFF;
		
		
	  //RCC_APB1PeriphClockCmd(LED3_EN,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = LED3_pin;
    GPIO_Init(LED3_gpio, &GPIO_InitStructure);

    LED3_OFF;
		

}

void GPIO_OverTurn(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	if(GPIO_ReadOutputDataBit(GPIOx,GPIO_Pin)==1)
		GPIO_ResetBits(GPIOx, GPIO_Pin);
		else if(GPIO_ReadOutputDataBit(GPIOx,GPIO_Pin)==0)
			GPIO_SetBits(GPIOx, GPIO_Pin);
}




