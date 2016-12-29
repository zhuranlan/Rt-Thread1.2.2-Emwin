

#include "rtthread.h"
#include "stm32f4xx_conf.h"
#include "GPIO.h"
extern volatile int OS_TimeMS;

void TIM3_Int_Init(u16 arr,u16 psc)
{
       TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
       NVIC_InitTypeDef NVIC_InitStructure;
      
       RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); 
      
       TIM_TimeBaseInitStructure.TIM_Period = arr; 
       TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  
       TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; 
       TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
      
       TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
      
       TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
      
       NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; 
       NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00; 
       NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; 
       NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
       NVIC_Init(&NVIC_InitStructure);
 
      TIM_Cmd(TIM3,ENABLE); 
}

void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)
	{
	   TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	   OS_TimeMS++;
		 GPIO_OverTurn(LED1_gpio,LED1_pin);
//	   if(OS_TimeMS%20==0)
//	   {
////	    GUI_TOUCH_Exec();
////			LED1=!LED1;
//	   }
	}
} 
