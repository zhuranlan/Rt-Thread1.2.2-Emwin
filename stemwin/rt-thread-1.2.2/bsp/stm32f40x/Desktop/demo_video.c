/*
*********************************************************************************************************
*	                                  
*	ģ������ : ��Ƶ����Ӧ��
*	�ļ����� : demo_video.c
*	��    �� : V1.0
*	˵    �� : ��Ƶ���ŵ���ʾ
*                                   
*	�޸ļ�¼ :
*		�汾��    ����          ����          ˵��
*		v1.0    2014-07-11    Eric2013        �׷�
*	
*	Copyright (C), 2014-2015, ���������� www.armfly.com
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
*	                         �Ի���GUI_WIDGET_CREATE_INFO������ֵ
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
*	�� �� ��: _GetMJPEGFileList
*	����˵��: ��ȡMJPEG�ļ��б�
*	��    �Σ�path  ·��
*	�� �� ֵ: ��
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

	/* ����Ӧ�ļ��� */
	res = f_opendir(&dir, path);
	if (res == FR_OK)
	{
		for (;;)
		{
			/* ��ȡĿ¼��������Զ����� */
			res = f_readdir(&dir, &fno);
			
			/* ������߳����˳� */
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
			
			/* �ж����ļ�������Ŀ¼ */
			if (fno.fattrib & AM_DIR)
			{
				continue;  /* Ŀ¼ */
			}
			else
			{
				/* �������Ƶ�ļ����ͽ��ļ������뵽�ļ��б� */
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
*	�� �� ��: goto_next_file
*	����˵��: ����һ����Ƶ�ļ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void goto_next_file (void)
{
	FRESULT res = FR_OK;

	/* FILEMGR_FileList.ptr����Ƶ�ļ�����������ļ������Ǵ���0�� */
	if(FILEMGR_FileList.ptr > 0)
	{
		/* �رյ�ǰ���ļ� */
		f_close(&Video_File);
		
		/* ��¼��ǰ�򿪵��ļ���� */
		FILEMGR_FileList.idx ++;

		/* ��ǰ�򿪵��ļ���Ŵ���FILEMGR_FileList.ptrʱ�ص���ʼλ�� */
		if (FILEMGR_FileList.idx == FILEMGR_FileList.ptr)
		{
			FILEMGR_FileList.idx = 0;
		}

		/* ���µ��ļ� */
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
*	�� �� ��: goto_previous_file
*	����˵��: ����һ����Ƶ�ļ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void goto_previous_file (void)
{
	FRESULT res = FR_OK;

	/* FILEMGR_FileList.ptr����Ƶ�ļ�����������ļ������Ǵ���0�� */
	if(FILEMGR_FileList.ptr > 0)
	{
		/* �رյ�ǰ���ļ� */
		f_close(&Video_File);  
		
		/* ��¼��ǰ�򿪵��ļ���� */
		FILEMGR_FileList.idx --;

		/* ��ǰ�򿪵��ļ����С��0ʱ�ص���ĩλ�� */
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
*	�� �� ��: _GetData
*	����˵��: called one time at the beginning to retrieve overhead
*			  information and, after this, several times to retrieve the actual MJPEG data
*	��    �Σ�p: application defined void pointer
*             ppData: pointer to the location the requested data resides in
*             NumBytesReq: number of requested bytes.
*             Off: defines the offset to use for reading the source data
*	�� �� ֵ: The number of bytes which were actually read (int)
*********************************************************************************************************
*/
static int _GetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off)
{
	unsigned int NumBytesRead;
	FIL * phFile;

	phFile = (FIL *)p;
	
	/*
	* ��⻺���С
	*/
	if (NumBytesReq > sizeof(_acVideoBuffer)) 
	{
		NumBytesReq = sizeof(_acVideoBuffer);
	}
	
	/*
	* ���ö�ȡλ��
	*/
	f_lseek(phFile, Off + ImageOffset);
	
	/*
	* ��ȡ���ݵ�������
	*/
	f_read(phFile, _acVideoBuffer, NumBytesReq, &NumBytesRead); 
	
	/*
	* ���û�ȡ����Ƶ���ݵ�
	*/
	*ppData =(unsigned char *) _acVideoBuffer;
	
	/*
	* ����ʵ�ʶ�ȡ���ݵ�����
	*/
	return NumBytesRead;  
}

/*
*********************************************************************************************************
*	�� �� ��: _PlayMJPEG
*	����˵��: draw the image after scaling it
*	��    �Σ�hFile: pointer to the MJPEG file
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static int _PlayMJPEG(FIL *hFile) 
{
	unsigned int  NumBytesRead;
	char *ptr;
	int timeout = 0xFFFF;
	static int counter = 0;

	/* ���ö�ȡλ�� */
	f_lseek(hFile, ImageOffset);

	t0 = GUI_GetTime();
	s0 = hFile->fptr;

	/* ���÷Ŵ� */
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

		/* ��ȡͼ��ı��˵�ַ */
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
*	�� �� ��: _cbVideoWindow
*	����˵��: Callback function of the MJPEG frame
*	��    �Σ�pMsg: pointer to a data structure of type WM_MESSAGE
*	�� �� ֵ: ��
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
		/* ������Ϣ */
		case  WM_TOUCH:
			/* ��Ƶ��ǰ����ʾ״̬ */
			if(VideoPlayer_State == VIDEO_PLAY)
			{
				pState = (const GUI_PID_STATE *)pMsg->Data.p;
				if (pState) 
				{
					/* �����ʾ���棬ʵ�ַŴ����С���л� */
					if (pState->Pressed == 1) 
					{
						VIDEO_Enlarge ^= 1;
						
						/* �����ǷŴ���ʾ */
						if(VIDEO_Enlarge)
						{
							GUI_SetOrientation(GUI_SWAP_XY | GUI_MIRROR_Y);
							TS_Orientation = 1;
							WM_AttachWindowAt(pMsg->hWin, WM_HBKWIN, 0, 0);
							WM_SetSize(pMsg->hWin, LCD_GetXSize(), LCD_GetYSize());
						}
						/* �ָ�ԭʼ��ʾ */
						else
						{
							/* 
							�������ڽ��Ӹ����ڷ��룬�����ӵ��µĸ����ڡ��¸������ڴ��������е���ԭ��
							����ɸ������ڴ��������еľ�ԭ����ͬ��
							*/
							WM_AttachWindow(pMsg->hWin, bkparent);
							GUI_SetOrientation(0);
							TS_Orientation = 0;
							WM_InvalidateWindow(bkparent);    
							
							/* �������ڽ��Ӹ����ڷ��룬�����ӵ��µĸ����ڡ�������λ�ý��������øô�����
							�����ڵĴ��������е�ԭ�㡣*/
							WM_AttachWindowAt(pMsg->hWin, VIDEO_hWin, 5, 16);
							WM_SetSize(pMsg->hWin,230, 170);
							do_clear = 1;
						}
					}
				}
			}
			break;
			
		/*������ʱ�� */
		case WM_CREATE:
			hTimerTime = WM_CreateTimer(pMsg->hWin, 0, 50, 0);        
			break;
		
		/* ��ʱ����Ϣ */
		case WM_TIMER:
			WM_InvalidateWindow(pMsg->hWin);
			WM_RestartTimer(pMsg->Data.v, 0);
			break; 

		/* �Ƴ���ʱ���ɾ����ʱ�� */
		case WM_DELETE:
			WM_DeleteTimer(hTimerTime);
			break;

		/* �ػ���Ϣ�Ĵ��� */
		case WM_PAINT:	
			
			GUI_SetBkColor(GUI_BLACK);
			GUI_Clear();
		
			/* ��ǰ����Ƶ����״̬ */
			if(VideoPlayer_State == VIDEO_PLAY)
			{
				/* ���������ϣ�������һ�� */
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

				/* ������ʾ���� */
				hItem = WM_GetDialogItem(WM_GetParent(pMsg->hWin), ID_VIDEO_PROGRESS);
				if (progress_bar == 0)
				{
					SLIDER_SetValue(hItem, (ImageOffset * 100 )/ Video_File.fsize);
				}

				/* ��ǰ����ʱ�� */
				hItem = WM_GetDialogItem(WM_GetParent(pMsg->hWin), ID_VIDEO_DURATION);
				duration = (GUI_GetTime() - time_start )/1000;
				if(duration < 0) duration = 0;
				sprintf(temp,"%02d:%02d", duration/60, duration%60); 
				TEXT_SetText(hItem, temp);  
			}
			/* ��ǰ����Ƶ��ͣ״̬ */
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
*	�� �� ��: _cbDialog
*	����˵��: �Ի���Ļص�����
*	��    �Σ�pMsg: WM_MESSAGE����ָ���ַ
*	�� �� ֵ: ��
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
			/* ��ʼ���Ի����ϵĿռ� */
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
			
			/* ������Ƶ״̬ */
			VideoPlayer_State = VIDEO_IDLE;
			
			/* �Ӹ�Ŀ¼�»�ȡ��Ƶ�ļ� */
			_GetMJPEGFileList("0:");

			/* ����text�ı� */
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

			/* ����Ƶ�ļ�������ʾ */
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
				/* �����ʼ���� */
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
			
			/* ������һ����Ƶ */
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
				
		/* �Ƴ���Ƶ���� */
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

		/* ��� */
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

		/* ������һ����Ƶ�ļ� */
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
			
		/* ����ֹͣ���Ű�ť */
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
*	�� �� ��: DEMO_Video
*	����˵��: videoӦ��
*	��    �Σ�hWin  ���ھ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DEMO_Video(WM_HWIN hWin)
{
	VIDEO_Enlarge = 0;
	bkparent = hWin;
	VIDEO_hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbDialog, hWin, 0, 0);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
