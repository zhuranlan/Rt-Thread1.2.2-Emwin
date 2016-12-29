/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <stdio.h>

#include "stm32f4xx.h"
#include <board.h>
#include <rtthread.h>
#include "GPIO.h"

#include "GUI.h"
#include "WM_Intern.h"



void 	Iwatchdog_init(void);
void STemwin_init(void);
//----------------------- Define --------------------------//
#define fpga_write(offset,data)	*((volatile unsigned short int *)(0x64000000 + ( offset<<1))) = data

#define fpga_read(offset)	*((volatile unsigned short int *)(0x64000000 + ( offset<<1)))
	

#define sram_write(offset,data)	*((volatile unsigned short int *)(0x6C000000 + ( offset<<1))) = data

#define sram_read(offset)	*((volatile unsigned short int *)(0x6C000000 + ( offset<<1)))

#define clock 400000000//400M时钟



void rt_init_thread_entry(void* parameter)
{
	   extern void rt_platform_init(void);
	
	    rt_platform_init();
		  Iwatchdog_init();
	    STemwin_init();

	

// while(1)
//	 rt_thread_delay(100);
	 
//FS

//GUI
}


#include "bsp_touch.h"
ALIGN(RT_ALIGN_SIZE)
static char thread_touch_stack[1024]  ;//__attribute__((at(0x6c000000)))= {0};
struct rt_thread thread_touch;
static void rt_thread_entry_touch(void* parameter)
{

    while(1)
    {

		if(g_tTP.Enable == 1)
			TOUCH_Scan();	

	GUI_TOUCH_Exec();	
		
	GPIO_OverTurn(LED1_gpio,LED1_pin);

  rt_thread_delay(10);
    }
}

void set_pwm1(unsigned int f,unsigned int duty)//输入频率 和占空比
{	
	    
	double temp;
	temp=f;
	temp=1/temp;
	temp=temp*clock;

			fpga_write(3,temp);
    	temp=(temp/10000)*duty;
		  fpga_write(4,temp);
	rt_kprintf("f:%d \n",fpga_read(3));
	rt_kprintf("duty:%d \n",fpga_read(4));

}

void set_pwm2(unsigned int f,unsigned int duty)
{
			fpga_write(5,f);
			fpga_write(6,duty);
	rt_kprintf("f:%d \n",fpga_read(5));
	rt_kprintf("duty:%d \n",fpga_read(6));

}


void  test_sram(void)
{
	   rt_int32_t i,max=512*1024, temp;
	  rt_uint16_t z=0;
		 /* SRAM  数据总线+地址总线测试*/
     {
			unsigned int check_flag=0;
			check_flag=0;
			for(i=0;i<max;i++)
			 {
				
			sram_write(i,z); 
				 z++;
			 }
			z=0;
			for(i=0;i<max;i++)
			{
				temp=sram_read(i);
				if(temp!=z)
				{
					check_flag=1;
			    rt_kprintf("%d \n",i<<1);
          rt_kprintf("sarm_read[%d]=%d error !! \n",i,temp);
				}
				z++;
			}
			if(check_flag==1)
				rt_kprintf("data bus & addr bus check erroe  !! \n");
			else if(check_flag==0)
				rt_kprintf("data bus & addr bus check successful !! \n");	
	   }
		 
		 
			/*16位数据通道测试*/
			{
			unsigned int check_flag=0;
			check_flag=0;
			for(i=0;i<65535;i++)
			{
			sram_write(0,i);
		  temp=sram_read(0);
			if(temp!=i)
			{
			check_flag=1;
			rt_kprintf("fpga_read[%d]=%d error!!\n",0,temp);
			}
			}
			if(check_flag==1)
				rt_kprintf("data bus check erroe  !! \n");
			else if(check_flag==0)
				rt_kprintf("data bus check successful !! \n ");
		 }
}

void tset_fpga_arm(void)
{
	   rt_int32_t i,max=256, temp;
					/* SRAM  数据总线+地址总线测试*/
     {
			unsigned int check_flag=0;
			check_flag=0;
			for(i=0;i<max;i++)
			fpga_write(i,i);
			
			for(i=0;i<max;i++)
			{
				temp=fpga_read(i);
				if(temp!=i)
				{
					check_flag=1;
			    rt_kprintf("%d \n",i);
          rt_kprintf("fpga_read[%d]=%d error !! \n",i,temp);
				}
			}
			if(check_flag==1)
				rt_kprintf("data bus & addr bus check erroe  !! \n");
			else if(check_flag==0)
				rt_kprintf("data bus & addr bus check successful !! \n");	
	   }
		 
		 
		 	/*16位数据通道测试*/
			{
			unsigned int check_flag=0;
			check_flag=0;
			for(i=0;i<65535;i++)
			{
			fpga_write(0,i);
		  fpga_write(1,i);
			fpga_write(2,i);
		  temp=fpga_read(0);
			if(temp!=i)
			{
			check_flag=1;
			rt_kprintf("fpga_read[%d]=%d error!!\n",0,temp);
			}
			}
			if(check_flag==1)
				rt_kprintf("data bus check erroe  !! \n");
			else if(check_flag==0)
				rt_kprintf("data bus check successful !! \n ");
		 }
			
}


ALIGN(RT_ALIGN_SIZE)
static char thread_emwin_stack[8192];
struct rt_thread thread_emwin;
extern void lcd_Initializtion(void);
extern void bsp_InitSPIBus(void);
static void rt_thread_entry_emwin(void* parameter)
{
	 lcd_Initializtion();
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	 bsp_InitSPIBus();
	 TOUCH_InitHard();
	 rt_thread_init(&thread_touch,
                   "touch",
                   rt_thread_entry_touch,
                   RT_NULL,
                   &thread_touch_stack[0],
                   sizeof(thread_touch_stack),15,20);
    rt_thread_startup(&thread_touch);

    //	GUI_Init();
    //	/* 启用多缓冲 */
    //	WM_MULTIBUF_Enable(1);
    // GUI_DispString("Hello STemWin!");
	
   //MainTask_Demo();
 	  WM_SetCreateFlags(WM_CF_MEMDEV);
   GUI_Init();
   GUIDEMO_Main();
   GUIDEMO_IconView();
}

void STemwin_init(void)
{    
	
	rt_thread_init(&thread_emwin,
                   "stemwin",
                   rt_thread_entry_emwin,
                   RT_NULL,
                   &thread_emwin_stack[0],
                   sizeof(thread_emwin_stack),29,20);
    rt_thread_startup(&thread_emwin);
}


#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(set_pwm1, set);
FINSH_FUNCTION_EXPORT(set_pwm2, set);
FINSH_FUNCTION_EXPORT(test_sram, set);
FINSH_FUNCTION_EXPORT(tset_fpga_arm, set);
#endif
int rt_application_init()
{
    rt_thread_t init_thread;

#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("app_init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("app_init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif

    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);
		

    return 0;
}


void Iwatchdog_Feed(void* parameter)
{

	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);//使能写入PR与RLR
  IWDG_SetPrescaler(IWDG_Prescaler_128);  //写入PR分屏
  IWDG_SetReload(1000);  //写入RLR
  IWDG_Enable();//KR写入0xCCCC
	rt_kprintf("Iwatch Dog Init ok ! \n");
	while(1)
	{
	IWDG_ReloadCounter();
  rt_thread_delay(200);
	GPIO_OverTurn(LED2_gpio,LED2_pin);
	}
}

static rt_uint8_t watchdog_stack[ 512 ];
static struct rt_thread watchdog;
void Iwatchdog_init(void)
{
	rt_err_t result;
	
		result = rt_thread_init(&watchdog,
		                    "watch_dog",
		                    Iwatchdog_Feed, 
							RT_NULL,
		                    &watchdog_stack[0],
							sizeof(watchdog_stack),
							13,
							20
							);

	if (result == RT_EOK)
        rt_thread_startup(&watchdog);
}

/*@}*/
