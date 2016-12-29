/*
*********************************************************************************************************
*	                                  
*	模块名称 : 启动界面
*	文件名称 : demo_startup.c
*	版    本 : V1.0
*	说    明 : 启动的演示
*                                   
*	修改记录 :
*		版本号    日期          作者          说明
*		v1.0    2014-07-11    Eric2013        首发
*	
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "DIALOG.h"
#include "startup_res.c"

/*
*********************************************************************************************************
*	                         extern 
*********************************************************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmlogo_armfly;
static uint32_t idx = 0;

/*
*********************************************************************************************************
*	函 数 名: _cbBk
*	功能说明: 桌面窗口的重绘函数
*	形    参：pMsg  WM_MESSAGE类型指针地址
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbBk(WM_MESSAGE * pMsg) 
{
	uint16_t   xPos, Step = 20, i;
	const GUI_BITMAP * pBm;

	switch (pMsg->MsgId) 
	{
		/* 桌面窗口的重绘 */
		case WM_PAINT:
			GUI_DrawGradientV(0, 0, LCD_GetXSize()-1, LCD_GetYSize() - 1, GUI_RED, GUI_YELLOW);   
			GUI_DrawBitmap(&bmlogo_armfly , (LCD_GetXSize() - bmlogo_armfly .XSize)/2 , (LCD_GetYSize() - bmlogo_armfly .YSize)/2); 

			for (i = 0, xPos = LCD_GetXSize() / 2 - 2 * Step; i < 5; i++, xPos += Step) 
			{
				pBm = (idx == i) ? &_bmWhiteCircle_10x10 : &_bmWhiteCircle_6x6;
				GUI_DrawBitmap(pBm, xPos - pBm->XSize / 2, 320 - pBm->YSize / 2);
			}
			
	}
}

/*
*********************************************************************************************************
*	函 数 名: DEMO_Starup
*	功能说明: GUI主函数
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void DEMO_Starup(void)
{
	uint8_t loop = 16 ;
	GUI_RECT Rect = {360, 315, 440, 325};

	/* 设置桌面窗口的回调函数 */
	WM_SetCallback(WM_HBKWIN, _cbBk);

	/* 设置启动界面的动态显示 */
	while (loop--)
	{
		idx = (16- loop) % 5;
		/* 让指定区域无效从而触发回调函数的WM_PAINT重绘消息 */
		WM_InvalidateArea(&Rect);
		GUI_Delay(200);
	}
}
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
