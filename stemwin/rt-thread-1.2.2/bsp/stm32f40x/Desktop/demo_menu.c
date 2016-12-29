/*
*********************************************************************************************************
*	                                  
*	ģ������ : �������
*	�ļ����� : MainTask.c
*	��    �� : V1.0
*	˵    �� : ����ͼ������
*                                   
*	�޸ļ�¼ :
*		�汾��    ����          ����          ˵��
*		v1.0    2014-07-11    Eric2013        �׷�
*	
*	Copyright (C), 2014-2015, ���������� www.armfly.com
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

/* ICONVIEW�ؼ�ͼ��ʹ�� */
static const BITMAP_ITEM _aBitmapItem[] = {
	{&bmmultimedia,   "Video Player"        , "Launch MPJPEG video"},
	{&bmimage,        "Image Browser"       , "Browse Images"},  
	{&bmclock,        "Clock/Calendar"      , "Clock settings"},
	{&bmgame,         "Game"                , "Launch Reversi game"},
	{&bmcpu,          "Perfomance"          , "Show CPU performance"},  
	{&bminfo,         "System Info"         , "Get System Information"},
};

/* ͼ���Ӧ��Ӧ�ó��� */
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
*	�� �� ��: _cbBk
*	����˵��: ���洰�ڻص�����
*	��    �Σ�pMsg  ָ���ַ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _cbBk(WM_MESSAGE * pMsg) 
{
	uint32_t NCode, Id, sel;
	static uint32_t module_mutex = 0;
	static uint8_t prevent_refresh = 0;

	switch (pMsg->MsgId) 
	{
		/* �ػ���Ϣ */
		case WM_PAINT:
//			if(prevent_refresh == 0)
//			{    
				GUI_DrawGradientV(0, 0, LCD_GetXSize()- 1, LCD_GetYSize() / 2, GUI_DARKBLUE, GUI_BLUE);
				GUI_DrawGradientV(0 ,LCD_GetYSize() / 2, LCD_GetXSize()- 1, LCD_GetYSize() - 1, GUI_BLUE, GUI_LIGHTBLUE);
				
				/* ����ͼ����ʾ��͸�� */
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
				/* ���ICONͼ��� */
				case WM_NOTIFICATION_CLICKED:
					prevent_refresh = 1;
					break;

				/* �ͷ�ICONͼ�� ͨ������prevent_refreshʵ�ֵ��ICONͼ���ͷź������ػ棬*/
				case WM_NOTIFICATION_RELEASED: 
					prevent_refresh = 0;
					if (Id == '0')
					{
						sel = ICONVIEW_GetSel(pMsg->hWinSrc);
						if(sel < GUI_COUNTOF(_aBitmapItem))
						{
							/* ͨ������module_mutex����ʵ��һ��ֻ�ܴ�һ��ͼ�꣬��ֹ����һ��ͼ��
							   ��ȥ����һ��ͼ�ꡣ
							*/
							if(module_mutex == 0)
							{
								module_mutex = 1;
								/* ������Ӧ��Ӧ�ó��� */
								_apModules [sel](pMsg->hWinSrc);
								current_module = sel;
							}
						}
					}
					break;
				
				/* ͨ�������Ϣ�ָ��޸ı���module_mutex = 0�Ϳ����ٴδ�ICONͼ�� */
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
*	�� �� ��: _cbStatus
*	����˵��: ����hStatusWin�Ļص�����
*	��    �Σ�pMsg  ָ���ַ
*	�� �� ֵ: ��
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
		
		/* �������ں�Ϊ�˴��ڴ���һ����ʱ�� */
		case WM_CREATE:
			hTimerTime = WM_CreateTimer(hWin, ID_TIMER_TIME, 1000, 0);
			break;
		
		/* ���ڱ�ɾ����ʱ��ͬʱɾ����ʱ�� */
		case WM_DELETE:
			WM_DeleteTimer(hTimerTime);
			break;
		
		/* ��ʱ����Ϣ����ʱ��ִ���ػ���Ϣ */
		case WM_TIMER:
			WM_InvalidateWindow(hWin);
			WM_RestartTimer(pMsg->Data.v, 0);
			break;

		/* �յ�USB�������Ϣ */
		case WM_MSG_USB_STATUS_CHANGED:    
			WM_InvalidateWindow(hWin);
			break;

		/* ִ���ػ� */
		case WM_PAINT:
			xSize = WM_GetWindowSizeX(hWin);
			ySize = WM_GetWindowSizeY(hWin);

			/* �Ȼ��Ʊ��� */
			GUI_SetColor(0x303030);
			GUI_FillRect(0, 0, xSize , ySize - 3);
			GUI_SetColor(0x808080);
			GUI_DrawHLine(ySize - 2, 0, xSize );
			GUI_SetColor(0x404040);
			GUI_DrawHLine(ySize - 1, 0, xSize );

			/* ��ʾʱ�� */
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

			/* ��������ͼ�� */
			if (alarm_set == 1)
			{
				GUI_DrawBitmap(&_bmAlarm_16x16, xSize - 73, 3);
			}

			/* ��ʾlogo  */
			GUI_DrawBitmap(&bmSTLogo40x20, 5, 1);

			/* ��ʾCPU������ */
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
*	�� �� ��: Demo Main menu
*	����˵��: ���˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DEMO_MainMenu(void) 
{
	ICONVIEW_Handle hIcon;
	WM_HWIN hStatusWin;
	int i = 0;

	/* ���ö໺�� */
	WM_MULTIBUF_Enable(1);

	/* ���洰�ڵĻص����� */
	WM_SetCallback(WM_HBKWIN, _cbBk);

	/* ����һ��״̬���� */
	hStatusWin = WM_CreateWindowAsChild(
									  0,              /* �������ڴ��������е�����Xλ�� */
									  0,              /* �������ڴ��������е�����Yλ�� */
									  LCD_GetXSize(), /* ���ڵ�X�ߴ硣���Ϊ0�����ø����ڿͻ�����X�ߴ� */
									  31,             /* ���ڵ�Y�ߴ硣���Ϊ0�����ø����ڿͻ�����Y�ߴ� */
									  WM_HBKWIN,      /* �����ڵľ�� */
									  WM_CF_SHOW|WM_CF_MEMDEV,     /* ���ڴ�����ʶ */
									  _cbStatus,      /* �ص����̵�ָ�룬��ʹ�ûص�ʱΪNULL */
									  0);             /* Ҫ����Ķ����ֽ�����ͨ��Ϊ0 */
	
	/* ����ICON�ؼ� */
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

	/* ����ICON�ؼ������� */
	ICONVIEW_SetFont(hIcon, &GUI_Font13B_ASCII);
	
	/* ����ICON�ؼ��ı��� */
	ICONVIEW_SetBkColor(hIcon, ICONVIEW_CI_SEL, 0x941000 | 0x80404040);

	/* ����ͼ���Y���� */
	ICONVIEW_SetSpace(hIcon, GUI_COORD_Y, 3);
	
	/* ����С���߱߿��ͼ��֮��Ŀ�ܴ�С */
	ICONVIEW_SetFrame(hIcon, GUI_COORD_Y, 1);

	/* ���ͼ�� */
	for (i = 0; i < GUI_COUNTOF(_aBitmapItem); i++)
	{
		ICONVIEW_AddBitmapItem(hIcon,_aBitmapItem[i].pBitmap, _aBitmapItem[i].pText);
	}

	/* ���þ۽� */
	WM_SetFocus(hIcon);

	while (1) 
	{
		GUI_Delay(100);
	}
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
