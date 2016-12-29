/*
 * File      : led.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

#ifndef __GPIO_H__
#define __GPIO_H__
#include <rtthread.h>
#include "sys.h"

#define ADM2587_1_EN                     RCC_AHB1Periph_GPIOA
#define ADM2587_1_gpio                   GPIOA
#define ADM2587_1_pin                   (GPIO_Pin_8)


#define ADM2587_2_EN                     RCC_AHB1Periph_GPIOB
#define ADM2587_2_gpio                   GPIOB
#define ADM2587_2_pin                   (GPIO_Pin_15)



#define LED1_EN                     RCC_AHB1Periph_GPIOF
#define LED1_gpio                   GPIOF
#define LED1_pin                   (GPIO_Pin_6)

#define LED2_EN                     RCC_AHB1Periph_GPIOI
#define LED2_gpio                   GPIOI
#define LED2_pin                   (GPIO_Pin_3)

#define LED3_EN                     RCC_AHB1Periph_GPIOI
#define LED3_gpio                   GPIOI
#define LED3_pin                   (GPIO_Pin_2)




#define RS485_1_RE GPIO_ResetBits(GPIOA, GPIO_Pin_8)
#define RS485_1_DE GPIO_SetBits(GPIOA, GPIO_Pin_8)

#define RS485_2_RE GPIO_ResetBits(GPIOB, GPIO_Pin_15)
#define RS485_2_DE GPIO_SetBits(GPIOB, GPIO_Pin_15)

#define LED1_ON GPIO_ResetBits(GPIOE, GPIO_Pin_1)
#define LED1_OFF GPIO_SetBits(GPIOE, GPIO_Pin_1)

#define LED2_ON GPIO_ResetBits(GPIOE, GPIO_Pin_0)
#define LED2_OFF GPIO_SetBits(GPIOE, GPIO_Pin_0)

#define LED3_ON GPIO_ResetBits(GPIOB, GPIO_Pin_9)
#define LED3_OFF GPIO_SetBits(GPIOB, GPIO_Pin_9)



#define	LED1 PFout(6)
#define	LED2 PIout(3)
#define	LED3 PIout(2)


void rt_hw_led_init(void);
void GPIO_OverTurn(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);



#endif
