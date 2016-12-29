/*
*********************************************************************************************************
*	                                  
*	模块名称 : 桌面设计
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : 桌面图标的设计
*                                   
*	修改记录 :
*		版本号    日期          作者          说明
*		v1.0    2014-07-11    Eric2013        首发
*	
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "WM.h"
#include "Res\menu_res.c"
#include "clock.c"
#include "cpu.c"
#include "game.c"
#include "image.c"
#include "info.c"
#include "multimedia.c"
#include "time_utils.h"
#include "cpu_utils.h"
#include "MainTask.h"

/*
*********************************************************************************************************
*	                         External variables
*********************************************************************************************************
*/
extern __IO uint8_t alarm_now;
extern __IO uint32_t alarm_set;
extern RTC_AlarmTypeDef  RTC_AlarmStructure;
extern WM_HWIN  VIDEO_hWin, hVideoScreen;
extern WM_HWIN  IMAGE_hWin, vFrame;
extern __IO uint32_t TS_Orientation;
extern __IO uint32_t IMAGE_Enlarge;
extern __IO uint32_t VIDEO_Enlarge;

extern void DEMO_SystemInfo ( WM_HWIN hWin);
extern void DEMO_Game(WM_HWIN hWin);
extern void DEMO_Video(WM_HWIN hWin);
extern void DEMO_Image(WM_HWIN hWin);
extern void DEMO_Clock(WM_HWIN hWin);
extern void DEMO_Cpu(WM_HWIN hWin);

/*
*********************************************************************************************************
*	                         Private typedef
*********************************************************************************************************
*/
typedef struct {
  const GUI_BITMAP * pBitmap;  
  const char       * pText;
  const char       * pExplanation;
} BITMAP_ITEM;

/*
*********************************************************************************************************
*	                         Private defines
*********************************************************************************************************
*/
#define WM_MSG_USB_STATUS_CHANGED      WM_USER + 0x01
#define ID_TIMER_TIME                  1

/*
*********************************************************************************************************
*	            Private macros   Private variables
*********************************************************************************************************
*/
uint32_t current_module = 0xFF;

/* ICONVIEW控件图标使用 */
static const BITMAP_ITEM _aBitmapItem[] = {
	{&bmmultimedia,   "Video Player"        , "Launch MPJPEG video"},
	{&bmimage,        "Image Browser"       , "Browse Images"},  
	{&bmclock,        "Clock/Calendar"      , "Clock settings"},
	{&bmgame,         "Game"                , "Launch Reversi game"},
	{&bmcpu,          "Perfomance"          , "Show CPU performance"},  
	{&bminfo,         "System Info"         , "Get System Information"},
};

/* 图标对应的应用程序 */
static void (* _apModules[])( WM_HWIN hWin) = 
{
	DEMO_Video,
	DEMO_Image,
	DEMO_Clock,
	DEMO_Game,
	DEMO_Cpu,
	DEMO_SystemInfo,
};

/*
*********************************************************************************************************
*	函 数 名: _cbBk
*	功能说明: 桌面窗口回调函数
*	形    参：pMsg  指针地址
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbBk(WM_MESSAGE * pMsg) 
{
	uint32_t NCode, Id, sel;
	static uint32_t module_mutex = 0;
	static uint8_t prevent_refresh = 0;

	switch (pMsg->MsgId) 
	{
		/* 重绘消息 */
		case WM_PAINT:
//			if(prevent_refresh == 0)
//			{    
				GUI_DrawGradientV(0, 0, LCD_GetXSize()- 1, LCD_GetYSize() / 2, GUI_DARKBLUE, GUI_BLUE);
				GUI_DrawGradientV(0 ,LCD_GetYSize() / 2, LCD_GetXSize()- 1, LCD_GetYSize() - 1, GUI_BLUE, GUI_LIGHTBLUE);
				
				/* 设置图标显示半透明 */
				GUI_SetAlpha(0x80);
				GUI_DrawBitmap(&bmlogo_armfly , (LCD_GetXSize() - bmlogo_armfly .XSize) / 2, ((LCD_GetYSize() - bmlogo_armfly .YSize) / 2) + 40); 
				GUI_SetAlpha(0x00);
//			}
			break;

		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);     
			NCode = pMsg->Data.v;  

			switch (NCode) 
			{  
				/* 点击ICON图标后 */
				case WM_NOTIFICATION_CLICKED:
					prevent_refresh = 1;
					break;

				/* 释放ICON图标 通过变量prevent_refresh实现点击ICON图标释放后再做重绘，*/
				case WM_NOTIFICATION_RELEASED: 
					prevent_refresh = 0;
					if (Id == '0')
					{
						sel = ICONVIEW_GetSel(pMsg->hWinSrc);
						if(sel < GUI_COUNTOF(_aBitmapItem))
						{
							/* 通过变量module_mutex可以实现一次只能打开一个图标，防止打开了一个图标
							   又去打开另一个图标。
							*/
							if(module_mutex == 0)
							{
								module_mutex = 1;
								/* 进入相应的应用程序 */
								_apModules [sel](pMsg->hWinSrc);
								current_module = sel;
							}
						}
					}
					break;
				
				/* 通过这个消息恢复修改变量module_mutex = 0就可以再次打开ICON图标 */
				case 0x500:
					module_mutex = 0;
					current_module = 0xFF;
					break;

				default:
					break;
			}
			break;
		default:
			WM_DefaultProc(pMsg);
	}
}

/*
*********************************************************************************************************
*	函 数 名: _cbStatus
*	功能说明: 窗口hStatusWin的回调函数
*	形    参：pMsg  指针地址
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbStatus(WM_MESSAGE * pMsg) 
{
	int xSize, ySize;
	static uint8_t TempStr[50];
	float CPU;

	RTC_TimeTypeDef   RTC_TimeStructure;
	RTC_DateTypeDef   RTC_DateStructure;
	uint8_t sec, min, hour;

	static WM_HTIMER hTimerTime;
	WM_HWIN hWin;

	hWin = pMsg->hWin;
	switch (pMsg->MsgId) 
	{
		case WM_PRE_PAINT:
			//GUI_MULTIBUF_Begin();
			break;
		
		case WM_POST_PAINT:
			//GUI_MULTIBUF_End();
			break;
		
		/* 创建窗口后为此窗口创建一个定时器 */
		case WM_CREATE:
			hTimerTime = WM_CreateTimer(hWin, ID_TIMER_TIME, 1000, 0);
			break;
		
		/* 窗口被删除的时候同时删除定时器 */
		case WM_DELETE:
			WM_DeleteTimer(hTimerTime);
			break;
		
		/* 定时器消息，定时器执行重绘消息 */
		case WM_TIMER:
			WM_InvalidateWindow(hWin);
			WM_RestartTimer(pMsg->Data.v, 0);
			break;

		/* 收到USB插入的消息 */
		case WM_MSG_USB_STATUS_CHANGED:    
			WM_InvalidateWindow(hWin);
			break;

		/* 执行重绘 */
		case WM_PAINT:
			xSize = WM_GetWindowSizeX(hWin);
			ySize = WM_GetWindowSizeY(hWin);

			/* 先绘制背景 */
			GUI_SetColor(0x303030);
			GUI_FillRect(0, 0, xSize , ySize - 3);
			GUI_SetColor(0x808080);
			GUI_DrawHLine(ySize - 2, 0, xSize );
			GUI_SetColor(0x404040);
			GUI_DrawHLine(ySize - 1, 0, xSize );

			/* 显示时间 */
			GUI_SetTextMode(GUI_TM_TRANS);
			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(GUI_FONT_16B_ASCII);

			RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
			sec    =  RTC_TimeStructure.RTC_Seconds;
			min    =  RTC_TimeStructure.RTC_Minutes;
			hour   =  RTC_TimeStructure.RTC_Hours;

			RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

			sprintf((char *)TempStr, "%02d:%02d:%02d", hour , min, sec);
			GUI_DispStringAt((char *)TempStr, xSize - 50, 4);

			/* 绘制闹钟图标 */
			if (alarm_set == 1)
			{
				GUI_DrawBitmap(&_bmAlarm_16x16, xSize - 73, 3);
			}

			/* 显示logo  */
			GUI_DrawBitmap(&bmSTLogo40x20, 5, 1);

			/* 显示CPU利用率 */
			CPU = (float)OSStatTaskCPUUsage/100;
			sprintf((char *)TempStr, "CPU : %5.2f %%", CPU);

			if(OSStatTaskCPUUsage < 7500 )
			{
				GUI_SetColor(GUI_WHITE);
			}
			else
			{
				GUI_SetColor(GUI_RED);
			}
			GUI_DispStringAt( (char *)TempStr, 50, 4);
			GUI_SetColor(GUI_WHITE);
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

/*
*********************************************************************************************************
*	函 数 名: Demo Main menu
*	功能说明: 主菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void DEMO_MainMenu(void) 
{
	ICONVIEW_Handle hIcon;
	WM_HWIN hStatusWin;
	int i = 0;

	/* 启用多缓冲 */
	WM_MULTIBUF_Enable(1);

	/* 桌面窗口的回调函数 */
	WM_SetCallback(WM_HBKWIN, _cbBk);

	/* 创建一个状态窗口 */
	hStatusWin = WM_CreateWindowAsChild(
									  0,              /* 父窗口在窗口坐标中的左上X位置 */
									  0,              /* 父窗口在窗口坐标中的左上Y位置 */
									  LCD_GetXSize(), /* 窗口的X尺寸。如果为0，则用父窗口客户区的X尺寸 */
									  31,             /* 窗口的Y尺寸。如果为0，则用父窗口客户区的Y尺寸 */
									  WM_HBKWIN,      /* 父窗口的句柄 */
									  WM_CF_SHOW|WM_CF_MEMDEV,     /* 窗口创建标识 */
									  _cbStatus,      /* 回调例程的指针，或不使用回调时为NULL */
									  0);             /* 要分配的额外字节数，通常为0 */
	
	/* 创建ICON控件 */
	hIcon = ICONVIEW_CreateEx(0, 
							  32, 
							  LCD_GetXSize(), 
							  LCD_GetYSize()- 33, 
							  WM_HBKWIN, 
							  WM_CF_SHOW | WM_CF_HASTRANS ,
							  ICONVIEW_CF_AUTOSCROLLBAR_V ,
							  '0', 
							  112, 
							  96);

	/* 设置ICON控件的字体 */
	ICONVIEW_SetFont(hIcon, &GUI_Font13B_ASCII);
	
	/* 设置ICON控件的背景 */
	ICONVIEW_SetBkColor(hIcon, ICONVIEW_CI_SEL, 0x941000 | 0x80404040);

	/* 设置图标的Y轴间距 */
	ICONVIEW_SetSpace(hIcon, GUI_COORD_Y, 3);
	
	/* 设置小工具边框和图标之间的框架大小 */
	ICONVIEW_SetFrame(hIcon, GUI_COORD_Y, 1);

	/* 添加图标 */
	for (i = 0; i < GUI_COUNTOF(_aBitmapItem); i++)
	{
		ICONVIEW_AddBitmapItem(hIcon,_aBitmapItem[i].pBitmap, _aBitmapItem[i].pText);
	}

	/* 设置聚焦 */
	WM_SetFocus(hIcon);

	while (1) 
	{
		GUI_Delay(100);
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
