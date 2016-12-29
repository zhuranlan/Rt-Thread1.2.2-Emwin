/*
*本文件主要用于RTT中不适合用rt_thread_delay进行延时程序中
*思路参考了正点原子中的delay.c，也是通过用抽取的方法不影响Systick
*
*
*
*/


#include "delay.h"
static rt_uint32_t systicks_per_us = 0;
static rt_uint32_t ms_per_rt_tick = 0;


void delay_init(void)
{
	systicks_per_us = (rt_uint32_t)(SystemCoreClock / 1000000);
	ms_per_rt_tick = (rt_uint32_t)(1000 / RT_TICK_PER_SECOND);
}


void delay_us(rt_uint32_t us)
{
	rt_uint32_t ticks;
	rt_uint32_t t_old, t_now, t_cnt = 0;
	rt_uint32_t reload = SysTick->LOAD;
	
	
	ticks = us * systicks_per_us;	
	while (1)
	{
		t_now = SysTick->VAL;
		if (t_now != t_old){
			if (t_now < t_old){
				t_cnt += t_old - t_now;
			}
			else{
				t_cnt += reload - t_now + t_old;
			}
			if (t_cnt >= ticks){
				break;
			}
		}
	}
}


void delay_ms(rt_uint16_t ms)
{
	rt_uint16_t tmp;
	
	if (ms >= ms_per_rt_tick){
		tmp = (rt_uint16_t)(ms/ms_per_rt_tick);
		rt_thread_delay(tmp);
		ms -= tmp;
	}
	if (ms > 0){
	delay_us(ms*1000);
	}
}

//--------------Delay_Nclk()---------------
void Delay_Nclk(u32 Nclk)
{
 while(Nclk)
 {
  Nclk--;
 }
}
//--------------Delay_Nus()---------------
u32 DelayOut;
static u8  Mult_us=0;
void Delay_Nus(u32 Nus)
{
 SysTick->LOAD=Nus*Mult_us;     
 SysTick->VAL=0x00;        
 SysTick->CTRL=0x01 ;       

 do
 {
  DelayOut=SysTick->CTRL;
 }
 while(DelayOut&0x01&&!(DelayOut&(1<<16)));	
  
 SysTick->CTRL=0x00;      
 SysTick->VAL =0X00;        
} 
//-------------Delay_Nms()-------------------
void Delay_Nms(u32 Nms)
{
 do
 {
  Delay_Nus(1000);
 }
 while(Nms--);
}

