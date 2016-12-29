/*
*********************************************************************************************************
*	                                  
*	模块名称 : 图片浏览应用
*	文件名称 : demo_imag.c
*	版    本 : V1.0
*	说    明 : 图片浏览的演示
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
#include "file_utils.h"
#include "str_utils.h"
#include "Res\common_res.h"
#include "string.h"
#include "MainTask.h"

/*
*********************************************************************************************************
*	                         External variables 
*********************************************************************************************************
*/
extern __IO uint32_t TS_Orientation;

/*
*********************************************************************************************************
*	                         Private typedef  Private defines
*********************************************************************************************************
*/
#define ID_FRAMEWIN_IMAGE   (GUI_ID_USER + 0x00)
#define ID_IMAGE_FILE       (GUI_ID_USER + 0x01)
#define PLAY_BUTTON     (GUI_ID_USER + 0x03)
#define PREV_BUTTON     (GUI_ID_USER + 0x04)
#define NEXT_BUTTON     (GUI_ID_USER + 0x05)
#define STOP_BUTTON     (GUI_ID_USER + 0x06)

/*
*********************************************************************************************************
*	                         对话框GUI_WIDGET_CREATE_INFO类型数组
*********************************************************************************************************
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Image Browser", ID_FRAMEWIN_IMAGE, 0, 0, 320, 240, WM_CF_STAYONTOP, 0},
  { TEXT_CreateIndirect, "No File Found", ID_IMAGE_FILE, 20, 2, 179, 23, 0, 0, 0 },
  { BUTTON_CreateIndirect, "", PREV_BUTTON, 20 , 177 , 80, 24, 0, 0, 0 },
  { BUTTON_CreateIndirect, "", NEXT_BUTTON, 116 , 177 , 80, 24, 0, 0, 0 },
  { BUTTON_CreateIndirect, "", STOP_BUTTON, 212 , 177 , 80, 24, 0, 0, 0 },
};

/*
*********************************************************************************************************
*	                         Private variables
*********************************************************************************************************
*/
static uint32_t _acBuffer[128];
FIL Image_File;
WM_HWIN  IMAGE_hWin, vFrame;
static WM_HWIN  bkparent ;
__IO uint32_t Image_browse_mode = 0;
__IO uint32_t IMAGE_Enlarge = 0;
__IO uint32_t Index;
__IO uint32_t new_display = 1;

/*
*********************************************************************************************************
*	函 数 名: _GetJPEGFileList
*	功能说明: 获取JPEG图片列表
*	形    参：path: pointer to an array of type char
*	返 回 值: 无
*********************************************************************************************************
*/
static void _GetJPEGFileList(char* path) 
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	char *fn;
#if _USE_LFN
	static char lfn[_MAX_LFN];
	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
#endif
	FILEMGR_FileList.ptr = 0;
	FILEMGR_FileList.idx = 0;

	res = f_opendir(&dir, path);
	if (res == FR_OK)
	{
		for (;;)
		{
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0)
			{
				break;
			}
			
			if (fno.fname[0] == '.')
			{
				continue;
			}
			
			#if _USE_LFN
				fn = *fno.lfname ? fno.lfname : fno.fname;
			#else
				fn = fno.fname;
			#endif
			
			if (fno.fattrib & AM_DIR)
			{
				continue;
			}
			else
			{
				if ((check_filename_ext(fn, "jpg"))|| (check_filename_ext(fn, "JPG")))
				{
					if (FILEMGR_FileList.ptr < FILELIST_DEPDTH)
					{
						strncpy((char *)FILEMGR_FileList.file[FILEMGR_FileList.ptr++].line, fn, 128);
					}
				}
			}
		}
	}
}

/*
*********************************************************************************************************
*	                          6种显示效果
*********************************************************************************************************
*/
/**
  * @brief  _EffectLineV0 transition effect
  * @param  xSize: x size of the image (int)
  *         ySize: y size of the image (int)
  *         Delay: transition delay (int)
  * @retval None
  */
static void _EffectLineV0(int xSize, int ySize, int Delay) {
  int i, j;
  
  GUI_SetColor(GUI_TRANSPARENT);
  for (i = 0; i < 32; i++)
  {
    for (j = i; j < ySize; j += 32)
    {
      GUI_DrawHLine(j, 0, xSize - 1);
    }
    GUI_X_Delay(Delay);
  }
}

/**
  * @brief  _EffectLineV1 transition effect
  * @param  xSize: x size of the image (int)
  *         ySize: y size of the image (int)
  *         Delay: transition delay (int)
  * @retval None
  */
static void _EffectLineV1(int xSize, int ySize, int Delay) {
  int i, j;
  
  GUI_SetColor(GUI_TRANSPARENT);
  for (i = 31; i >= 0; i--)
  {
    for (j = i; j < ySize; j += 32)
    {
      GUI_DrawHLine(j, 0, xSize - 1);
    }
    GUI_X_Delay(Delay);
  }
}

/**
  * @brief  _EffectLineH0 transition effect
  * @param  xSize: x size of the image (int)
  *         ySize: y size of the image (int)
  *         Delay: transition delay (int)
  * @retval None
  */
static void _EffectLineH0(int xSize, int ySize, int Delay) {
  int i, j;
  
  GUI_SetColor(GUI_TRANSPARENT);
  for (i = 0; i < 32; i++)
  {
    for (j = i; j < xSize; j += 32)
    {
      GUI_DrawVLine(j, 0, ySize - 1);
    }
    GUI_X_Delay(Delay);
  }
}

/**
  * @brief  _EffectLineH1 transition effect
  * @param  xSize: x size of the image (int)
  *         ySize: y size of the image (int)
  *         Delay: transition delay (int)
  * @retval None
  */
static void _EffectLineH1(int xSize, int ySize, int Delay) {
  int i, j;
  
  GUI_SetColor(GUI_TRANSPARENT);
  for (i = 31; i >= 0; i--)
  {
    for (j = i; j < xSize; j += 32)
    {
      GUI_DrawVLine(j, 0, ySize - 1);
    }
    GUI_X_Delay(Delay);
  }
}

/**
  * @brief  _EffectLineHV0 transition effect
  * @param  xSize: x size of the image (int)
  *         ySize: y size of the image (int)
  *         Delay: transition delay (int)
  * @retval None
  */
static void _EffectLineHV0(int xSize, int ySize, int Delay) {
  int i, j;
  
  GUI_SetColor(GUI_TRANSPARENT);
  for (i = 0; i < 32; i++) {
    for (j = i; j < ySize; j += 32)
    {
      GUI_DrawHLine(j, 0, xSize - 1);
    }
    for (j = i; j < xSize; j += 32)
    {
      GUI_DrawVLine(j, 0, ySize - 1);
    }
    GUI_X_Delay(Delay);
  }
}

/**
  * @brief  _EffectLineHV1 transition effect
  * @param  xSize: x size of the image (int)
  *         ySize: y size of the image (int)
  *         Delay: transition delay (int)
  * @retval None
  */
static void _EffectLineHV1(int xSize, int ySize, int Delay) {
  int i, j;
  
  GUI_SetColor(GUI_TRANSPARENT);
  
  for (i = 31; i >= 0; i--) {
    for (j = i; j < ySize; j += 32)
    {
      GUI_DrawHLine(j, 0, xSize - 1);
    }
    for (j = i; j < xSize; j += 32)
    {
      GUI_DrawVLine(j, 0, ySize - 1);
    }
    GUI_X_Delay(Delay);
  }
}

/* Array of effect functions */
static void (* _apFunc[])(int xSize, int ySize, int Delay) = 
{
	_EffectLineV0,
	_EffectLineV1,
	_EffectLineH0,
	_EffectLineH1,
	_EffectLineHV0,
	_EffectLineHV1,
};

/*
*********************************************************************************************************
*	函 数 名: goto_next_file
*	功能说明: Display the next image in the list
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void goto_next_file (void)
{
	FRESULT res = FR_OK;

	if(FILEMGR_FileList.ptr > 0)
	{
		f_close(&Image_File);
		FILEMGR_FileList.idx ++;

		if (FILEMGR_FileList.idx == FILEMGR_FileList.ptr)
		{
			FILEMGR_FileList.idx = 0;
		}

		res = f_open(&Image_File, (char const *)FILEMGR_FileList.file[FILEMGR_FileList.idx ].line, FA_OPEN_EXISTING | FA_READ);
		if ( res != FR_OK)
		{
			/* Error open file */
		}
	}
	else
	{
		/* No file */
	} 
}

/*
*********************************************************************************************************
*	函 数 名: goto_previous_file
*	功能说明: Display the previous image in the list
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void goto_previous_file (void)
{
	FRESULT res = FR_OK;

	if(FILEMGR_FileList.ptr > 0)
	{
		f_close(&Image_File); 
		FILEMGR_FileList.idx --;

		if (FILEMGR_FileList.idx < 0 )
		{
			FILEMGR_FileList.idx = FILEMGR_FileList.ptr - 1;
		}

		res = f_open(&Image_File, (char const *)FILEMGR_FileList.file[FILEMGR_FileList.idx ].line, FA_OPEN_EXISTING | FA_READ);
		if ( res != FR_OK)
		{
			/* Error open file */
		}
	}
	else
	{
		/* no file */
	} 
}

/**
  * @brief  called one time at the beginning to retrieve overhead
  *         information and, after this, several times to retrieve the actual image data
  * @param  p: application defined void pointer
  *         ppData: pointer to the location the requested data resides in
  *         NumBytesReq: number of requested bytes.
  *         Off: defines the offset to use for reading the source data
  * @retval The number of bytes which were actually read (int)
  */
static int _GetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off)
{
	unsigned int NumBytesRead;
	FIL * phFile;

	phFile = (FIL *)p;
	/*
	* Check buffer size
	*/
	if (NumBytesReq > sizeof(_acBuffer)) 
	{
		NumBytesReq = sizeof(_acBuffer);
	}
	/*
	* Set file pointer to the required position
	*/
	f_lseek(phFile, Off);
	/*
	* Read data into buffer
	*/
	f_read(phFile, _acBuffer, NumBytesReq, &NumBytesRead); 
	/*
	* Set data pointer to the beginning of the buffer
	*/
	*ppData =(unsigned char *) _acBuffer;
	/*
	* Return number of available bytes
	*/
	return NumBytesRead;  
}

/**
  * @brief  draw the image after scaling it
  * @param  hFile: pointer to the image file
  *         Enlarge: 0 = window mode
  *                  1 = full screen mode
  * @retval None
  */
static void _DrawJPEG(FIL *hFile, uint8_t Enlarge) 
{
	int XPos, YPos, XSize, YSize, nx, ny, n;
	GUI_JPEG_INFO Info;
	GUI_JPEG_GetInfoEx(_GetData, hFile, &Info);

	if((Info.XSize == 0) || (Info.YSize == 0))
	{
		return;
	}
	if(  Enlarge == 0)
	{
		nx = 275000 / Info.XSize;
		ny = 167000 / Info.YSize;
	}
	else
	{
		nx = 320000 / Info.XSize;
		ny = 240000 / Info.YSize;
	}

	if (nx < ny)
	{
		n = nx;
	} 
	else 
	{
		n = ny;
	}

	XSize = Info.XSize * n / 1000;
	YSize = Info.YSize * n / 1000;
	if(Enlarge == 0)
	{
		XPos = (275 - XSize) / 2;
		YPos = (167 - YSize) / 2;
	}
	else
	{
		XPos = (320 - XSize) / 2;
		YPos = (240 - YSize) / 2;
	}

	GUI_JPEG_DrawScaledEx(_GetData, hFile, XPos, YPos, n, 1000);
}

/**
  * @brief  show one effect amongst the effect list
  * @param  pFunc: array of effect functions
  *         xSize: x size of the image
  *         ySize: y size of the image
  *         Delay: transition delay
  * @retval None
  */
static void _ShowEffect(void (* pFunc)(int xSize, int ySize, int Delay), int xSize, int ySize, int Delay)
{
	pFunc(xSize, ySize, Delay);
}

/*
*********************************************************************************************************
*	函 数 名: _cbImageWindow
*	功能说明: Callback function of the image frame
*	形    参：pMsg: pointer to a data structure of type WM_MESSAGE
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbImageWindow(WM_MESSAGE * pMsg) 
{
	GUI_RECT r;
	const GUI_PID_STATE * pState;
	WM_HWIN hItem;
	static WM_HTIMER hTimerTime;  

	switch (pMsg->MsgId) 
	{
		/* 触摸消息 */
		case  WM_TOUCH:
			pState = (const GUI_PID_STATE *)pMsg->Data.p;
			if (pState) 
			{
				/* 实现图片的全屏显示与否 */
				if (pState->Pressed == 1)
				{
					IMAGE_Enlarge ^= 1;
					new_display = 1;
					if(IMAGE_Enlarge)
					{
						WM_AttachWindowAt(pMsg->hWin, WM_HBKWIN, 0, 0);
						WM_SetSize(pMsg->hWin,LCD_GetXSize(), LCD_GetYSize());
					}
					else
					{
						WM_AttachWindowAt(pMsg->hWin, IMAGE_hWin, 25, 35);
						WM_SetSize(pMsg->hWin,267, 165);
					}
					WM_InvalidateWindow(pMsg->hWin);
				}
			}
			break;
    
		 case WM_CREATE:
			/* Create timer */
			hTimerTime = WM_CreateTimer(pMsg->hWin, 0, 3000, 0);
			break;
    
		case WM_NOTIFY_VIS_CHANGED:
			break;
    
		/* 重绘消息 */
		case WM_PAINT:
			WM_GetInsideRect(&r);
		    /* 图片首次显示好之后才开始显示效果的处理 */
			if (!new_display)
			{
				_ShowEffect(_apFunc[Index++], r.x1, r.y1, 25);
				Index = (Index == GUI_COUNTOF(_apFunc)) ? 0 : Index;
			}
			GUI_ClearRectEx(&r);
			_DrawJPEG(&Image_File, IMAGE_Enlarge);
			new_display = 0;
			break;
    
		/* 定时器消息 每3s切换到下一张 */
		case WM_TIMER:
			if (Image_browse_mode)
			{
				f_close(&Image_File);
				goto_next_file();
				hItem = WM_GetDialogItem(WM_GetParent(pMsg->hWin), ID_IMAGE_FILE);
				TEXT_SetText(hItem, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line);
				WM_InvalidateWindow(pMsg->hWin);
			}
			WM_RestartTimer(pMsg->Data.v, 0);
			break;
			
		/* 删除对话框的时候，将创建的定时器也删除 */
		case WM_DELETE:
			WM_DeleteTimer(hTimerTime);
			break;
    
		default:
			WM_DefaultProc(pMsg);
  }
  
}

/*
*********************************************************************************************************
*	函 数 名: _cbDialog
*	功能说明: Callback routine of the dialog
*	形    参：pMsg: pointer to a data structure of type WM_MESSAGE
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbDialog(WM_MESSAGE * pMsg) 
{
	WM_HWIN hItem, hClient;
	GUI_RECT r;
	int Id, NCode;

	switch (pMsg->MsgId) 
	{
		case WM_INIT_DIALOG:
			WM_AttachWindow(pMsg->hWin, WM_HBKWIN);
			//     GUI_SetOrientation(GUI_SWAP_XY | GUI_MIRROR_Y);
			//     TS_Orientation = 1;
			hItem = pMsg->hWin;

			FRAMEWIN_SetFont       (hItem, &GUI_Font24B_ASCII);
			FRAMEWIN_SetTextAlign  (hItem, GUI_TA_HCENTER);
			FRAMEWIN_AddCloseButton(hItem, FRAMEWIN_BUTTON_LEFT,  0);

			/* 对话框上穿件的子窗口用图片的显示 */
			hClient = WM_GetClientWindow(pMsg->hWin);
			WM_GetClientRectEx(hClient, &r);
			vFrame = WM_CreateWindowAsChild(r.x0 + 20, r.y0 + 18, r.x1 - 40, r.y1 - 50, hClient, WM_CF_SHOW, _cbImageWindow , 0);

			hItem = WM_GetDialogItem(pMsg->hWin, PREV_BUTTON);     
			BUTTON_SetBitmapEx(hItem, 0, &bmButtonPreviousOn, 32, 6);
			BUTTON_SetBitmapEx(hItem, 1, &bmButtonPreviousOff,32, 6);

			hItem = WM_GetDialogItem(pMsg->hWin, NEXT_BUTTON);     
			BUTTON_SetBitmapEx(hItem, 0, &bmButtonNextOn, 32, 6);
			BUTTON_SetBitmapEx(hItem, 1, &bmButtonNextOff,32, 6); 

			hItem = WM_GetDialogItem(pMsg->hWin, STOP_BUTTON);    
			Image_browse_mode = 0; 
			BUTTON_SetText(hItem, "Manual");
			BUTTON_SetTextColor(hItem, BUTTON_CI_PRESSED,GUI_LIGHTBLUE);
			BUTTON_SetTextColor(hItem, BUTTON_CI_UNPRESSED,GUI_LIGHTBLUE);

			/* 获取根目录下所有图片的路径 */
			_GetJPEGFileList("0:");

			hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_FILE);   
			if (FILEMGR_FileList.ptr == 0)
			{
				TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
				TEXT_SetTextColor(hItem, GUI_BROWN);
			}
			else
			{
				TEXT_SetFont(hItem, GUI_FONT_13B_ASCII);
				TEXT_SetTextColor(hItem, 0x00804000);
			}

			if (FILEMGR_FileList.ptr > 0)
			{
				hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_FILE);
				TEXT_SetText(hItem, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line);
				if(f_open(&Image_File, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line, FA_OPEN_EXISTING | FA_READ) != FR_OK)
				{
					/* Error to read file */
				}
			}
			else
			{
				/* No file */
			}
			break;
		
		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);
			NCode = pMsg->Data.v;

			/* 删除子窗口时的消息 */
			if(NCode == WM_NOTIFICATION_CHILD_DELETED)
			{
				f_close(&Image_File);
				WM_AttachWindow(pMsg->hWin, bkparent);
				GUI_SetOrientation(0);
				TS_Orientation = 0;
				WM_InvalidateWindow(bkparent);
				WM_NotifyParent(WM_GetParent(pMsg->hWin), 0x500);      
				break; 
			}   
    
			switch(Id) 
			{
				/* 点击上一幅按钮 */
				case PREV_BUTTON: /* Notifications sent by 'Button' */
					switch(NCode) 
					{
						case WM_NOTIFICATION_CLICKED:
							break;
						case WM_NOTIFICATION_RELEASED:
							hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_FILE);     
							f_close(&Image_File);
							goto_previous_file();
							TEXT_SetText(hItem, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line);           
							WM_InvalidateWindow(vFrame);
							break;
					}
					break;
				
				/* 点击下一副 */
				case NEXT_BUTTON: /* Notifications sent by 'Button' */
					switch(NCode) 
					{
						case WM_NOTIFICATION_CLICKED:
							break;
						case WM_NOTIFICATION_RELEASED:
							hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_FILE);
							f_close(&Image_File);
							goto_next_file();
							TEXT_SetText(hItem, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line);        
							WM_InvalidateWindow(vFrame);
							break;
					}
					break;
					
				case STOP_BUTTON: /* Notifications sent by 'Button' */
					switch(NCode) 
					{
						case WM_NOTIFICATION_CLICKED:
							break;
						
						/* 用于实现自动和手动模式的切换 */
						case WM_NOTIFICATION_RELEASED:
							Image_browse_mode ^= 1; 
							if(Image_browse_mode == 1)
							{
								BUTTON_SetText(pMsg->hWinSrc, "Automatic");
								BUTTON_SetTextColor(pMsg->hWinSrc, BUTTON_CI_PRESSED,GUI_RED);
								BUTTON_SetTextColor(pMsg->hWinSrc, BUTTON_CI_UNPRESSED,GUI_RED);
							}
							else
							{
								BUTTON_SetText(pMsg->hWinSrc, "Manual");
								BUTTON_SetTextColor(pMsg->hWinSrc, BUTTON_CI_PRESSED,GUI_LIGHTBLUE);
								BUTTON_SetTextColor(pMsg->hWinSrc, BUTTON_CI_UNPRESSED,GUI_LIGHTBLUE);
							}
							break;
					}
					break;
			}
			break;
    
		case WM_DELETE:
			new_display = 1;
			break;
		default:
			WM_DefaultProc(pMsg);
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: DEMO_Image
*	功能说明: Image应用
*	形    参：hWin: Window handle
*	返 回 值: 无
*********************************************************************************************************
*/
void DEMO_Image(WM_HWIN hWin)
{
	IMAGE_Enlarge = 0;
	bkparent = hWin;
	IMAGE_hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbDialog, hWin, 0, 0);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
