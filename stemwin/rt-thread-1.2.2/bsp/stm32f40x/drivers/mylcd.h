#ifndef __MYLCD_H__
#define __MYLCD_H__

#include <rtthread.h>
#include "stm32f4xx.h"
//#include <finsh.h>
#include "delay.h"
#include "sys.h"

#define USE_HORIZONTAL 1

#define LCD_DATA    ((u32)0x68020000)    	//disp Data ADDR		   算这个比较坑爹
#define LCD_CMD     ((u32)0x68000000)	 		//disp Reg ADDR
	 
//-----------------LCD端口定义---------------- 
#define	LCD_LED PBout(14) //LCD背光    		 PB14 	    
	 
//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左	 

#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向

/*---------------------- Graphic LCD size definitions ------------------------*/
#if USE_HORIZONTAL==1	//使用横屏
#define LCD_WIDTH 480
#define LCD_HEIGHT 320
#else
#define LCD_WIDTH 320
#define LCD_HEIGHT 480
#endif
#define BPP             16                  /* Bits per pixel                     */
#define BYPP            ((BPP+7)/8)         /* Bytes per pixel                    */


/* LCD color */
#define White            0xFFFF
#define Black            0x0000
#define Grey             0xF7DE
#define Blue             0x001F
#define Blue2            0x051F
#define Red              0xF800
#define Magenta          0xF81F
#define Green            0x07E0
#define Cyan             0x7FFF
#define Yellow           0xFFE0



//=================================================================================
//9320/9325 LCD寄存器  
#define R0             0x00
#define R1             0x01
#define R2             0x02
#define R3             0x03
#define R4             0x04
#define R5             0x05
#define R6             0x06
#define R7             0x07
#define R8             0x08
#define R9             0x09
#define R10            0x0A
#define R12            0x0C
#define R13            0x0D
#define R14            0x0E
#define R15            0x0F
#define R16            0x10
#define R17            0x11
#define R18            0x12
#define R19            0x13
#define R20            0x14
#define R21            0x15
#define R22            0x16
#define R23            0x17
#define R24            0x18
#define R25            0x19
#define R26            0x1A
#define R27            0x1B
#define R28            0x1C
#define R29            0x1D
#define R30            0x1E
#define R31            0x1F
#define R32            0x20
#define R33            0x21
#define R34            0x22
#define R36            0x24
#define R37            0x25
#define R40            0x28
#define R41            0x29
#define R43            0x2B
#define R45            0x2D
#define R48            0x30
#define R49            0x31
#define R50            0x32
#define R51            0x33
#define R52            0x34
#define R53            0x35
#define R54            0x36
#define R55            0x37
#define R56            0x38
#define R57            0x39
#define R59            0x3B
#define R60            0x3C
#define R61            0x3D
#define R62            0x3E
#define R63            0x3F
#define R64            0x40
#define R65            0x41
#define R66            0x42
#define R67            0x43
#define R68            0x44
#define R69            0x45
#define R70            0x46
#define R71            0x47
#define R72            0x48
#define R73            0x49
#define R74            0x4A
#define R75            0x4B
#define R76            0x4C
#define R77            0x4D
#define R78            0x4E
#define R79            0x4F
#define R80            0x50
#define R81            0x51
#define R82            0x52
#define R83            0x53
#define R96            0x60
#define R97            0x61
#define R106           0x6A
#define R118           0x76
#define R128           0x80
#define R129           0x81
#define R130           0x82
#define R131           0x83
#define R132           0x84
#define R133           0x85
#define R134           0x86
#define R135           0x87
#define R136           0x88
#define R137           0x89
#define R139           0x8B
#define R140           0x8C
#define R141           0x8D
#define R143           0x8F
#define R144           0x90
#define R145           0x91
#define R146           0x92
#define R147           0x93
#define R148           0x94
#define R149           0x95
#define R150           0x96
#define R151           0x97
#define R152           0x98
#define R153           0x99
#define R154           0x9A
#define R157           0x9D
#define R192           0xC0
#define R193           0xC1
#define R229           0xE5					

extern struct rt_device_graphic_ops lcd_ili_ops;

void LCD_WR_REG(u16 regval);
void LCD_WR_DATA(u16 data);
void LCD_DrawPoint(u16 x,u16 y,u16 POINT_COLOR);
uint16_t get_pixel(int x, int y);
 void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2,u16 POINT_COLOR);
 uint16_t LCD_GetHeight(void);
 uint16_t LCD_GetWidth(void);
//=========================================================================
void lcd_Initializtion(void);
void rt_hw_lcd_set_pixel(const char* pixel, int x, int y);
void rt_hw_lcd_get_pixel(char* pixel, int x, int y);
void rt_hw_lcd_draw_hline(const char* pixel, int x1, int x2, int y);
void rt_hw_lcd_draw_vline(const char* pixel, int x, int y1, int y2);
void rt_hw_lcd_draw_blit_line(const char* pixels, int x, int y, rt_size_t size);

//液晶底层驱动和rtGUI之间的接口
static rt_err_t lcd_init(rt_device_t dev);
static rt_err_t lcd_open(rt_device_t dev, rt_uint16_t oflag);
static rt_err_t lcd_close(rt_device_t dev);
static rt_err_t lcd_control(rt_device_t dev, rt_uint8_t cmd, void *args);
void rt_hw_lcd_init(void);

#endif
