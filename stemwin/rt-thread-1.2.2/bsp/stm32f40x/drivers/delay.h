#ifndef __DELAY_H__
#define __DELAY_H__

#include <board.h>
#include <rtthread.h>
#include <stm32f4xx.h>


void delay_init(void);
void delay_ms(rt_uint16_t ms);
void delay_us(rt_uint32_t us);
//--------------Delay_Nclk()---------------
void Delay_Nclk(u32 Nclk);
//--------------Delay_Nus()---------------
void Delay_Nus(u32 Nus);
//-------------Delay_Nms()-------------------
void Delay_Nms(u32 Nms);

#endif

