/*
*********************************************************************************************************
*	                                  
*	模块名称 : 视频播放应用
*	文件名称 : demo_video.c
*	版    本 : V1.0
*	说    明 : 视频播放的演示
*                                   
*	修改记录 :
*		版本号    日期          作者          说明
*		v1.0    2014-07-11    Eric2013        首发
*	
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/


#include "GUI_JPEG_Private.h"
#include "file_utils.h"
#include "str_utils.h"
#include "common_res.h"
#include "string.h"

/*
*********************************************************************************************************
*	                         External variables 
*********************************************************************************************************
*/
extern __IO uint32_t TS_Orientation;


/*
*********************************************************************************************************
*	                         Private typedef Private defines
*********************************************************************************************************
*/
#define ID_FRAMEWIN_VIDEO   (GUI_ID_USER + 0x00)
#define PLAY_BUTTON_VIDEO   (GUI_ID_USER + 0x03)
#define PREV_BUTTON_VIDEO   (GUI_ID_USER + 0x04)
#define NEXT_BUTTON_VIDEO   (GUI_ID_USER + 0x05)
#define STOP_BUTTON_VIDEO   (GUI_ID_USER + 0x06)
#define ID_VIDEO_FILE       (GUI_ID_USER + 0x07)
#define ID_VIDEO_FPS        (GUI_ID_USER + 0x08)
#define ID_VIDEO_PROGRESS   (GUI_ID_USER + 0x09)
#define ID_VIDEO_DURATION   (GUI_ID_USER + 0x0A)
#define ID_VIDEO_EXIT       (GUI_ID_USER + 0x0B)

#define VIDEO_IDLE      0
#define VIDEO_PLAY      1
#define VIDEO_STOP      2
#define VIDEO_PAUSE     3

#define VIDEO_BUFFER_SIZE   (512)

/*
*********************************************************************************************************
*	                         Private macros Private variables
*********************************************************************************************************
*/
__IO uint32_t VIDEO_Enlarge = 0;
WM_HWIN  VIDEO_hWin, hVideoScreen;
FIL Video_File;
static WM_HWIN  bkparent ;

char _acVideoBuffer[VIDEO_BUFFER_SIZE];
uint8_t flag = 0;
__IO int ImageOffset = 0;
uint8_t VideoPlayer_State = VIDEO_IDLE;
int t0, frame_average_period, frame_speed;
int s0, frame_average_size;
static int progress_bar = 0;
int duration = 0;
int do_clear = 0;
int time_start, current_time, time_pause, time_seek;

/*
*********************************************************************************************************
*	                         对话框GUI_WIDGET_CREATE_INFO类型数值
*********************************************************************************************************
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "MJPEG Player", ID_FRAMEWIN_VIDEO, 0, 0, 240, 320-20, WM_CF_STAYONTOP, 0},
  { TEXT_CreateIndirect, "No File Found", ID_VIDEO_FILE, 10, 190, 160, 32, 0, 0, 0 },
  { SLIDER_CreateIndirect, NULL, ID_VIDEO_PROGRESS, 2, 180, 240-4-12, 10 },
  { TEXT_CreateIndirect, "00:00", ID_VIDEO_DURATION  , 190-12, 190, 179, 32, 0, 0, 0 },
  { BUTTON_CreateIndirect, "", PLAY_BUTTON_VIDEO, 29 + 58, 207 + 26, 48, 24, 0, 0, 0 },
  { BUTTON_CreateIndirect, "", PREV_BUTTON_VIDEO, 29 + 8 , 207 + 26, 48, 24, 0, 0, 0 },
  { BUTTON_CreateIndirect, "", NEXT_BUTTON_VIDEO, 29 + 108 , 207 + 26, 48, 24, 0, 0, 0 },
  { BUTTON_CreateIndirect, "", STOP_BUTTON_VIDEO, 29 + 58 , 207, 48, 24, 0, 0, 0 },//
  { BUTTON_CreateIndirect, "Exit", ID_VIDEO_EXIT, 29 + 58 , 207 + 52, 48, 24, 0, 0, 0 },  
};

/*
*********************************************************************************************************
*	函 数 名: _GetMJPEGFileList
*	功能说明: 获取MJPEG文件列表
*	形    参：path  路径
*	返 回 值: 无
*********************************************************************************************************
*/
static void _GetMJPEGFileList(char* path) 
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

	/* 打开相应文件夹 */
	res = f_opendir(&dir, path);
	if (res == FR_OK)
	{
		for (;;)
		{
			/* 读取目录项，索引会自动下移 */
			res = f_readdir(&dir, &fno);
			
			/* 读完或者出错退出 */
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
			
			/* 判断是文件还是子目录 */
			if (fno.fattrib & AM_DIR)
			{
				continue;  /* 目录 */
			}
			else
			{
				/* 如果是视频文件，就将文件名存入到文件列表 */
				if ((check_filename_ext(fn, "mjpeg"))|| \
				(check_filename_ext(fn, "MJPEG"))|| \
				(check_filename_ext(fn, "MJP")))
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
*	函 数 名: goto_next_file
*	功能说明: 打开下一个视频文件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void goto_next_file (void)
{
	FRESULT res = FR_OK;

	/* FILEMGR_FileList.ptr是视频文件数量，如果文件数量是大于0的 */
	if(FILEMGR_FileList.ptr > 0)
	{
		/* 关闭当前的文件 */
		f_close(&Video_File);
		
		/* 记录当前打开的文件标号 */
		FILEMGR_FileList.idx ++;

		/* 当前打开的文件标号大于FILEMGR_FileList.ptr时回到起始位置 */
		if (FILEMGR_FileList.idx == FILEMGR_FileList.ptr)
		{
			FILEMGR_FileList.idx = 0;
		}

		/* 打开新的文件 */
		res = f_open(&Video_File, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx ].line, FA_OPEN_EXISTING | FA_READ);
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
*	功能说明: 打开上一个视频文件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void goto_previous_file (void)
{
	FRESULT res = FR_OK;

	/* FILEMGR_FileList.ptr是视频文件数量，如果文件数量是大于0的 */
	if(FILEMGR_FileList.ptr > 0)
	{
		/* 关闭当前的文件 */
		f_close(&Video_File);  
		
		/* 记录当前打开的文件标号 */
		FILEMGR_FileList.idx --;

		/* 当前打开的文件标号小于0时回到最末位置 */
		if (FILEMGR_FileList.idx < 0 )
		{
			FILEMGR_FileList.idx = FILEMGR_FileList.ptr - 1;
		}

		res = f_open(&Video_File, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx ].line, FA_OPEN_EXISTING | FA_READ);
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
*	函 数 名: _GetData
*	功能说明: called one time at the beginning to retrieve overhead
*			  information and, after this, several times to retrieve the actual MJPEG data
*	形    参：p: application defined void pointer
*             ppData: pointer to the location the requested data resides in
*             NumBytesReq: number of requested bytes.
*             Off: defines the offset to use for reading the source data
*	返 回 值: The number of bytes which were actually read (int)
*********************************************************************************************************
*/
static int _GetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off)
{
	unsigned int NumBytesRead;
	FIL * phFile;

	phFile = (FIL *)p;
	
	/*
	* 检测缓存大小
	*/
	if (NumBytesReq > sizeof(_acVideoBuffer)) 
	{
		NumBytesReq = sizeof(_acVideoBuffer);
	}
	
	/*
	* 设置读取位置
	*/
	f_lseek(phFile, Off + ImageOffset);
	
	/*
	* 读取数据到缓存中
	*/
	f_read(phFile, _acVideoBuffer, NumBytesReq, &NumBytesRead); 
	
	/*
	* 设置获取到音频数据的
	*/
	*ppData =(unsigned char *) _acVideoBuffer;
	
	/*
	* 返回实际读取数据的数量
	*/
	return NumBytesRead;  
}

/*
*********************************************************************************************************
*	函 数 名: _PlayMJPEG
*	功能说明: draw the image after scaling it
*	形    参：hFile: pointer to the MJPEG file
*	返 回 值: 无
*********************************************************************************************************
*/
static int _PlayMJPEG(FIL *hFile) 
{
	unsigned int  NumBytesRead;
	char *ptr;
	int timeout = 0xFFFF;
	static int counter = 0;

	/* 设置读取位置 */
	f_lseek(hFile, ImageOffset);

	t0 = GUI_GetTime();
	s0 = hFile->fptr;

	/* 设置放大 */
	if(VIDEO_Enlarge == 1)
	{
		GUI_JPEG_DrawEx(_GetData, hFile, 0, 0);
	}
	else
	{
		GUI_JPEG_DrawScaledEx(_GetData, hFile, 7, 2, 2, 3);  
	}

	if(counter ++ > 20 )
	{
		frame_average_size =  (hFile->fptr - s0 + frame_average_size)/2; 
		frame_average_period = (GUI_GetTime() - t0 + frame_average_period)/2;
		counter = 0;
	}

	if(frame_average_size != 0)
	{
		frame_speed = frame_average_size / frame_average_period ;
	}	
	else
	{
		frame_speed = 70;
	}
	
	ImageOffset = hFile->fptr;

	while ( --timeout > 0 )
	{
		f_lseek(hFile, ImageOffset - VIDEO_BUFFER_SIZE);
		f_read(hFile, (char *)_acVideoBuffer, VIDEO_BUFFER_SIZE, &NumBytesRead); 

		/* 获取图像的便宜地址 */
		ptr = _acVideoBuffer;
		while (NumBytesRead --)
		{
			if((*ptr == 0xFF) && ((*(ptr + 1)) == M_SOI))
			{
				ImageOffset -=  (NumBytesRead + 1) ;
				return 0;
			}
			ptr++;
		}
		ImageOffset +=  2 * VIDEO_BUFFER_SIZE; 

		if (ImageOffset >= hFile->fsize)
		{
			return -1;
		}

	}
	
	return -1;
}

/*
*********************************************************************************************************
*	函 数 名: _cbVideoWindow
*	功能说明: Callback function of the MJPEG frame
*	形    参：pMsg: pointer to a data structure of type WM_MESSAGE
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbVideoWindow(WM_MESSAGE * pMsg) 
{
	char temp[20];
	static WM_HTIMER hTimerTime;
	WM_HWIN hItem;  
	const GUI_PID_STATE * pState;

	GUI_RECT r;
	WM_GetInsideRect(&r);

	switch (pMsg->MsgId) 
	{
		/* 触摸消息 */
		case  WM_TOUCH:
			/* 视频当前是演示状态 */
			if(VideoPlayer_State == VIDEO_PLAY)
			{
				pState = (const GUI_PID_STATE *)pMsg->Data.p;
				if (pState) 
				{
					/* 点击显示界面，实现放大和缩小的切换 */
					if (pState->Pressed == 1) 
					{
						VIDEO_Enlarge ^= 1;
						
						/* 这里是放大显示 */
						if(VIDEO_Enlarge)
						{
							GUI_SetOrientation(GUI_SWAP_XY | GUI_MIRROR_Y);
							TS_Orientation = 1;
							WM_AttachWindowAt(pMsg->hWin, WM_HBKWIN, 0, 0);
							WM_SetSize(pMsg->hWin, LCD_GetXSize(), LCD_GetYSize());
						}
						/* 恢复原始显示 */
						else
						{
							/* 
							给定窗口将从父窗口分离，并附加到新的父窗口。新父窗口在窗口坐标中的新原点
							将与旧父窗口在窗口坐标中的旧原点相同。
							*/
							WM_AttachWindow(pMsg->hWin, bkparent);
							GUI_SetOrientation(0);
							TS_Orientation = 0;
							WM_InvalidateWindow(bkparent);    
							
							/* 给定窗口将从父窗口分离，并附加到新的父窗口。给定的位置将用于设置该窗口在
							父窗口的窗口坐标中的原点。*/
							WM_AttachWindowAt(pMsg->hWin, VIDEO_hWin, 5, 16);
							WM_SetSize(pMsg->hWin,230, 170);
							do_clear = 1;
						}
					}
				}
			}
			break;
			
		/*创建定时器 */
		case WM_CREATE:
			hTimerTime = WM_CreateTimer(pMsg->hWin, 0, 50, 0);        
			break;
		
		/* 定时器消息 */
		case WM_TIMER:
			WM_InvalidateWindow(pMsg->hWin);
			WM_RestartTimer(pMsg->Data.v, 0);
			break; 

		/* 推出的时候得删除定时器 */
		case WM_DELETE:
			WM_DeleteTimer(hTimerTime);
			break;

		/* 重绘消息的处理 */
		case WM_PAINT:	
			
			GUI_SetBkColor(GUI_BLACK);
			GUI_Clear();
		
			/* 当前是视频播放状态 */
			if(VideoPlayer_State == VIDEO_PLAY)
			{
				/* 如果播放完毕，播放下一个 */
				if(_PlayMJPEG(&Video_File) < 0)
				{
					VideoPlayer_State = VIDEO_PLAY;
					ImageOffset = 0; 
					time_start = GUI_GetTime();    
					f_close(&Video_File);
					goto_next_file();      
					hItem = WM_GetDialogItem(WM_GetParent(pMsg->hWin), ID_VIDEO_FILE);
					TEXT_SetText(hItem, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line);        
				}

				/* 设置显示进度 */
				hItem = WM_GetDialogItem(WM_GetParent(pMsg->hWin), ID_VIDEO_PROGRESS);
				if (progress_bar == 0)
				{
					SLIDER_SetValue(hItem, (ImageOffset * 100 )/ Video_File.fsize);
				}

				/* 当前播放时间 */
				hItem = WM_GetDialogItem(WM_GetParent(pMsg->hWin), ID_VIDEO_DURATION);
				duration = (GUI_GetTime() - time_start )/1000;
				if(duration < 0) duration = 0;
				sprintf(temp,"%02d:%02d", duration/60, duration%60); 
				TEXT_SetText(hItem, temp);  
			}
			/* 当前是视频暂停状态 */
			else if(VideoPlayer_State == VIDEO_IDLE)
			{
				GUI_ClearRectEx(&r);
			}
			break;

		default:
		WM_DefaultProc(pMsg);
	}
}

/*
*********************************************************************************************************
*	函 数 名: _cbDialog
*	功能说明: 对话框的回调函数
*	形    参：pMsg: WM_MESSAGE类型指针地址
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbDialog(WM_MESSAGE * pMsg) 
{
	WM_HWIN hItem, hClient;
	GUI_RECT r;
	int Id, NCode, offset;

	switch (pMsg->MsgId) 
	{
		case WM_INIT_DIALOG:
			/* 初始化对话框上的空间 */
			hItem = WM_GetDialogItem(pMsg->hWin, ID_VIDEO_DURATION);   
			TEXT_SetFont(hItem, GUI_FONT_13HB_ASCII);
			TEXT_SetTextColor(hItem, 0x00804000);   

			hClient = WM_GetClientWindow(pMsg->hWin);
			WM_GetClientRectEx(hClient, &r);
			hVideoScreen = WM_CreateWindowAsChild(r.x0 + 2, r.y0 + 2, r.x1 - 2, r.y1 - 118, hClient, WM_CF_SHOW, _cbVideoWindow , 0);
			hItem = WM_GetDialogItem(pMsg->hWin, PLAY_BUTTON_VIDEO);

			BUTTON_SetBitmapEx(hItem, 0, &bmButtonPlayPauseOn, 16, 6);
			BUTTON_SetBitmapEx(hItem, 1, &bmButtonPlayPauseOff,16, 6);

			hItem = WM_GetDialogItem(pMsg->hWin, PREV_BUTTON_VIDEO);     
			BUTTON_SetBitmapEx(hItem, 0, &bmButtonPreviousOn, 16, 6);
			BUTTON_SetBitmapEx(hItem, 1, &bmButtonPreviousOff,16, 6);

			hItem = WM_GetDialogItem(pMsg->hWin, NEXT_BUTTON_VIDEO);     
			BUTTON_SetBitmapEx(hItem, 0, &bmButtonNextOn, 16, 6);
			BUTTON_SetBitmapEx(hItem, 1, &bmButtonNextOff,16, 6); 

			hItem = WM_GetDialogItem(pMsg->hWin, STOP_BUTTON_VIDEO);     
			BUTTON_SetBitmapEx(hItem, 0, &bmButtonStopOn, 16, 6);
			BUTTON_SetBitmapEx(hItem, 1, &bmButtonStopOff,16, 6); 

			hItem = WM_GetDialogItem(pMsg->hWin, ID_VIDEO_PROGRESS);       
			SLIDER_SetRange(hItem, 0, 100);
			SLIDER_SetWidth( hItem, 5);
			
			/* 设置视频状态 */
			VideoPlayer_State = VIDEO_IDLE;
			
			/* 从根目录下获取视频文件 */
			_GetMJPEGFileList("0:");

			/* 设置text文本 */
			hItem = WM_GetDialogItem(pMsg->hWin, ID_VIDEO_FILE);   
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

			do_clear = 1;

			/* 有视频文件可以显示 */
			if (FILEMGR_FileList.ptr > 0)
			{
				if(f_open(&Video_File, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line, FA_OPEN_EXISTING | FA_READ) == FR_OK)
				{
					hItem = WM_GetDialogItem(pMsg->hWin, ID_VIDEO_FILE);
					TEXT_SetText(hItem, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line);            
					VideoPlayer_State = VIDEO_PLAY;
					ImageOffset = 0;
					time_start = GUI_GetTime();
					hItem = WM_GetDialogItem(pMsg->hWin, PLAY_BUTTON_VIDEO);
					BUTTON_SetBitmapEx(hItem, 0, &bmButtonPlay, 16, 6);
					BUTTON_SetBitmapEx(hItem, 1, &bmButtonPlayPauseOff,16, 6);
				}
				else
				{
					/* error */
				}
			}
			break;

		case WM_PAINT:
            			
			break;

		case  WM_NOTIFY_CHILD_HAS_FOCUS:
			do_clear = 1;
			break;

		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);
			NCode = pMsg->Data.v;
			if(NCode == WM_NOTIFICATION_CHILD_DELETED)
			{
				f_close(&Video_File);
				WM_NotifyParent(WM_GetParent(pMsg->hWin), 0x500);      
				break; 
			}   

			switch(Id) 
			{
				/* 点击开始播放 */
				case PLAY_BUTTON_VIDEO: // Notifications sent by 'Button'
					switch(NCode) 
					{
						case WM_NOTIFICATION_RELEASED:
						if(VideoPlayer_State == VIDEO_IDLE)
						{
							if (FILEMGR_FileList.ptr > 0)
							{
								if(f_open(&Video_File, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line, FA_OPEN_EXISTING | FA_READ) == FR_OK)
								{
									hItem = WM_GetDialogItem(pMsg->hWin, ID_VIDEO_FILE);
									TEXT_SetText(hItem, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line);            
									VideoPlayer_State = VIDEO_PLAY;
									ImageOffset = 0;
									time_start = GUI_GetTime();
									hItem = WM_GetDialogItem(pMsg->hWin, PLAY_BUTTON_VIDEO);
									BUTTON_SetBitmapEx(hItem, 0, &bmButtonPlay, 16, 6);
									BUTTON_SetBitmapEx(hItem, 1, &bmButtonPlayPauseOff,16, 6);
								}
								else
								{
									/* error */
								}
							}
							else
							{
								/* No file */
							}
						}
						else if(VideoPlayer_State == VIDEO_PLAY)
						{
							time_pause = GUI_GetTime();
							hItem = WM_GetDialogItem(pMsg->hWin, PLAY_BUTTON_VIDEO);
							BUTTON_SetBitmapEx(hItem, 0, &bmButtonPause, 16, 6);
							BUTTON_SetBitmapEx(hItem, 1, &bmButtonPlayPauseOff,16, 6);
							VideoPlayer_State = VIDEO_PAUSE;
						}
						else if(VideoPlayer_State == VIDEO_PAUSE)
						{
							time_start = GUI_GetTime()+ time_start - time_pause ;
							hItem = WM_GetDialogItem(pMsg->hWin, PLAY_BUTTON_VIDEO);
							BUTTON_SetBitmapEx(hItem, 0, &bmButtonPlay, 16, 6);
							BUTTON_SetBitmapEx(hItem, 1, &bmButtonPlayPauseOff,16, 6);
							VideoPlayer_State = VIDEO_PLAY;
						}        
						break;
				}
				break;
			
			/* 播放上一个视频 */
			case PREV_BUTTON_VIDEO: /* Notifications sent by 'Button' */
				switch(NCode) 
				{
					case WM_NOTIFICATION_RELEASED:
						if(VideoPlayer_State != VIDEO_IDLE)
						{
							ImageOffset = 0;  
							time_start = GUI_GetTime();
							VideoPlayer_State = VIDEO_PLAY;
							f_close(&Video_File);
							goto_previous_file();
							hItem = WM_GetDialogItem(pMsg->hWin, ID_VIDEO_FILE);
							TEXT_SetText(hItem, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line);   
							hItem = WM_GetDialogItem(pMsg->hWin, PLAY_BUTTON_VIDEO);
							BUTTON_SetBitmapEx(hItem, 0, &bmButtonPlay, 16, 6);
							BUTTON_SetBitmapEx(hItem, 1, &bmButtonPlayPauseOff,6, 6);
						}
						break;
				}
				break;
				
		/* 推出视频播放 */
		case ID_VIDEO_EXIT:
			switch(NCode) 
			{
				case WM_NOTIFICATION_RELEASED:
				{
					GUI_EndDialog(pMsg->hWin, 0);
				}
				break;        
			}
			break;

		/* 快进 */
		case ID_VIDEO_PROGRESS: 
			switch(NCode) 
			{

				case WM_NOTIFICATION_CLICKED:
					{
						progress_bar = 1;
						hItem = WM_GetDialogItem(pMsg->hWin, ID_VIDEO_PROGRESS);
						ImageOffset = (Video_File.fsize * SLIDER_GetValue(hItem))/100;
						offset =ImageOffset - Video_File.fptr;
						time_start -= (offset / frame_speed);
					}
					break;
				
				case WM_NOTIFICATION_RELEASED:
					progress_bar = 0;
					break;
			}
			break;

		/* 播放下一个视频文件 */
		case NEXT_BUTTON_VIDEO: /* Notifications sent by 'Button' */
			switch(NCode) 
			{
				case WM_NOTIFICATION_RELEASED:
					if(VideoPlayer_State != VIDEO_IDLE)
					{       
						ImageOffset = 0; 
						VideoPlayer_State = VIDEO_PLAY;          
						time_start = GUI_GetTime();
						f_close(&Video_File);
						goto_next_file();      
						hItem = WM_GetDialogItem(pMsg->hWin, ID_VIDEO_FILE);
						TEXT_SetText(hItem, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line);
						hItem = WM_GetDialogItem(pMsg->hWin, PLAY_BUTTON_VIDEO);
						BUTTON_SetBitmapEx(hItem, 0, &bmButtonPlay, 16, 6);
						BUTTON_SetBitmapEx(hItem, 1, &bmButtonPlayPauseOff,16, 6);   
					}     
					break;
			}
			break;
			
		/* 按下停止播放按钮 */
		case STOP_BUTTON_VIDEO: /* Notifications sent by 'Button' */
			switch(NCode) 
			{
				case WM_NOTIFICATION_RELEASED:
					VideoPlayer_State = VIDEO_IDLE;
					hItem = WM_GetDialogItem(pMsg->hWin, PLAY_BUTTON_VIDEO);
					BUTTON_SetBitmapEx(hItem, 0, &bmButtonPlayPauseOn, 16, 6);
					BUTTON_SetBitmapEx(hItem, 1, &bmButtonPlayPauseOff,16, 6);     
					break;
			}
			break;
		}
		break;
	default:
		WM_DefaultProc(pMsg);
		break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: DEMO_Video
*	功能说明: video应用
*	形    参：hWin  窗口句柄
*	返 回 值: 无
*********************************************************************************************************
*/
void DEMO_Video(WM_HWIN hWin)
{
	VIDEO_Enlarge = 0;
	bkparent = hWin;
	VIDEO_hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbDialog, hWin, 0, 0);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
