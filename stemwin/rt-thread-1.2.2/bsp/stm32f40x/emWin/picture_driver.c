

#include "picture_driver.h"

//	static char _acBuffer[0x2000];
	static char _acBuffer[20];
	#define Picture_Buffer 100*1024
	GUI_JPEG_INFO JpegInfo;
	GUI_GIF_INFO InfoGif1;
	GUI_GIF_IMAGE_INFO InfoGif2;

/*
*********************************************************************************************************
*	�� �� ��: _GetData
*	����˵��: ������GUI_BMP_DrawEx()����
*	��    �Σ�p             FIL��������
*             NumBytesReq   �����ȡ���ֽ���
*             ppData        ����ָ��
*             Off           ���Off = 1����ô�����´���ʵλ�ö�ȡ                 
*	�� �� ֵ: ���ض�ȡ���ֽ���
*********************************************************************************************************
*/
static int _GetData(int  fd_p,const U8 ** ppData, unsigned NumBytesReq, U32 Off) 
{
	U32 i;
	
	static int FileAddress = 0;

	/*
	* ��⻺���С
	*/
	if (NumBytesReq > sizeof(_acBuffer)) {
	NumBytesReq = sizeof(_acBuffer);
	}

	/*
	* ���ö�ȡλ��
	*/
	if(Off == 1) FileAddress = 0;
	else FileAddress = Off;
	
  lseek(fd_p, FileAddress,SEEK_SET);

	
	/*
	* ��ȡ���ݵ�����, ����FatFS+�ٷ�SD���ķ�������DMA�����ϵ�4�ֽڶ������⣬������С�ڵ���һ��
	* SD��������С������������512�ֽڻ����
	*/
	for(i = 0; i < NumBytesReq / 512; i++)
	{
		read(fd_p, &_acBuffer[512*i], 512);
	}
	
	 read(fd_p, &_acBuffer[512*i], NumBytesReq % 512);


	/*
	* ��ָ��ppDataָ���ȡ�ĺ���
	*/
	*ppData = (const U8 *)&_acBuffer[0];

	/*
	* ���ض�ȡ���ֽ���
	*/
	return NumBytesReq;
}


/*
*********************************************************************************************************
*	�� �� ��: _GetData PNG
*	����˵��: ������GUI_PNG_DrawEx����
*	��    ��: p             FIL��������
*             NumBytesReq   �����ȡ���ֽ���
*             ppData        ����ָ��
*             Off           ���Off = 1����ô�����´���ʵλ�ö�ȡ                 
*	�� �� ֵ: ���ض�ȡ���ֽ���
*********************************************************************************************************
*/
static int _GetData_PNG(int fd_p, const U8 ** ppData, unsigned NumBytesReq, U32 Off) 
{
	static int FileAddress = 0;


	U8     * pData;

	pData  = (U8 *)*ppData;

	
	//
	// �������ݶ�ȡλ��
	//
	if(Off == 1) FileAddress = 0;
	else FileAddress = Off;
	lseek(fd_p, FileAddress,SEEK_SET);
	//
	// ��ȡ���ݵ�����
	//
 read(fd_p, pData, NumBytesReq);

	//
	// ���ض�ȡ��С
	//
	return NumBytesReq;

}

/*
*********************************************************************************************************
*	�� �� ��: _ShowBMPEx
*	����˵��: ��ʾBMPͼƬ
*	��    ��: sFilename Ҫ��ʾͼƬ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void _ShowBMPEx(const char * sFilename, U16 usPOSX, U16 usPOSY) 
{	

   int fd;
	
    /* ���ļ� */		
	fd = open(sFilename, O_RDONLY,0);
	
	if (fd < 0)
	{
		rt_kprintf("��ͼƬʧ�ܣ����\\n");
		return;
	}
	
//	XSize = GUI_BMP_GetXSizeEx(_GetData, &file);
//	YSize = GUI_BMP_GetYSizeEx(_GetData, &file);


	  GUI_BMP_DrawEx(_GetData,(int *)fd, usPOSX, usPOSY);
	
	  close(fd);
}




/*
*********************************************************************************************************
*	�� �� ��: _ShowJPGEx
*	����˵��: ��ʾJPEGͼƬ
*	��    �Σ�sFilename Ҫ��ʾͼƬ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
 void _ShowJPGEx(const char * sFilename, U16 usPOSX, U16 usPOSY) 
{	

	unsigned int i;
	int fd;

	/* ���ļ� */		
	fd = open(sFilename,  O_RDONLY,0);
	if (fd < 0)
	{
		rt_kprintf("��ͼƬʧ�ܣ����\\n");
		return;
	}
	
	GUI_JPEG_GetInfoEx(_GetData, (int *)fd, &JpegInfo);
	//while(1)
	{
		/**********��Ҫ��ʱ�򣬿��Լ��ϵ���������ֹˢͼƬ��ʱ������*************/
		//for(i = 100; i < 130; i += 10)
//		{

//			GUI_JPEG_DrawScaledEx(_GetData, 
//								  (int *)fd, 
//							      (LCD_GetXSize() - JpegInfo.XSize*i/100)/2,
//							      (LCD_GetYSize() - JpegInfo.YSize*i/100)/2,
//								  i,
//								  100);

//			//GUI_Delay(1000);	
//		}
		
		GUI_JPEG_DrawEx(_GetData,(int *)fd,usPOSX,usPOSY);

		GUI_Clear();
		GUI_Delay(1);	
	}

	close(fd);
}

/*
*********************************************************************************************************
*	�� �� ��: _ShowGIFEx
*	����˵��: GIFͼƬ��ʾ
*	��    ��: sFilename �ļ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
 void _ShowGIFEx(const char * sFilename) 
{	

	unsigned int i,j;
	int fd;

	/* ���ļ� */		
	  fd=open( sFilename,  O_RDONLY,0);
	if (fd < 0)
	{
		rt_kprintf("��ͼƬʧ�ܣ����\\n");
		return;
	}

	/* ��ȡGIFͼƬ��Ϣ */
	GUI_GIF_GetInfoEx(_GetData, (int *)fd,&InfoGif1);
	j=85;
	while(j--)
	{
		if(i < InfoGif1.NumImages)
	    {
		
	        GUI_GIF_GetImageInfoEx(_GetData, (int *)fd, &InfoGif2, i );
	        GUI_GIF_DrawSubEx(_GetData, 
			                 ( int *)fd, 
			                  (LCD_GetXSize() - InfoGif1.xSize)/2,
							  (LCD_GetYSize() - InfoGif1.ySize)/2, 
			                  i++);
     	
			//GUI_X_Delay(InfoGif2.Delay);											 
	    }
	    else
	    {
	        i = 0;
	    }
	}

   close(fd);
}


/*
*********************************************************************************************************
*	�� �� ��: _ShowPNG
*	����˵��: ��ʾPNGͼƬ
*	��    ��: sFilename Ҫ��ʾ��ͼƬ����
*             usPOSX    ��ʾλ��X
*             usPOSY    ��ʾλ��Y
*	�� �� ֵ: ��
*********************************************************************************************************
*/
 void _ShowPNGEx(const char * sFilename, U16 usPOSX, U16 usPOSY) 
{	
int fd;
	/* ���ļ� */				
	fd=open( sFilename,  O_RDONLY,0);
	if (fd < 0)
	{
		rt_kprintf("��ͼƬʧ�ܣ����\\n");
		return;
	}
	
	GUI_PNG_DrawEx(_GetData_PNG, (int *)fd, usPOSX, usPOSY);

	close(fd);
}



/*
*********************************************************************************************************
*	�� �� ��: _ShowBMP
*	����˵��: ��ʾBMPͼƬ  ���ص�SRAM����ʾ
*	��    ��: sFilename Ҫ��ʾ��ͼƬ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void _ShowBMP_RAM(const char * sFilename) 
{
	int XSize, YSize;
	GUI_HMEM hMem;
	char *_acBuffer2;
	int i;
	int fd;

	/* ����һ���ڴ�ռ� ���ҽ������� */
    hMem = GUI_ALLOC_AllocZero(Picture_Buffer);
	/* �����뵽�ڴ�ľ��ת����ָ������ */
	_acBuffer2 = GUI_ALLOC_h2p(hMem);

	
    /* ���ļ� */		
	fd = open(sFilename, O_RDONLY,0);
	
	if (fd < 0)
	{
		rt_kprintf("��ͼƬʧ�ܣ����\\n");
		return;
	}
	
	/* ��ȡͼƬ�������� */
	 read(fd,_acBuffer2, Picture_Buffer);


	XSize = GUI_BMP_GetXSize(_acBuffer2);
	YSize = GUI_BMP_GetYSize(_acBuffer2);

	while(1)
	{
		for(i = 100; i < 600; i += 10)
		{
			/* ������ʾ */
			GUI_BMP_DrawScaled(_acBuffer2, 
							   (LCD_GetXSize() - XSize*i/100)/2, 
							   (LCD_GetYSize() - YSize*i/100)/2, 
							   i, 
							   100);
			//GUI_Delay(3000);	
		}
		GUI_Clear();			
	}

	GUI_ALLOC_Free(hMem);
	close(fd);
}


/*
*********************************************************************************************************
*	�� �� ��: _ShowPNG
*	����˵��: ��ʾGIFƬ   ���ص�SRAM����ʾ
*	��    �Σ�sFilename Ҫ��ʾ��ͼƬ����
*             usPOSX    ��ʾλ��X
*             usPOSY    ��ʾλ��Y
*	�� �� ֵ: ��
*********************************************************************************************************
*/
 void _ShowPNG_RAM(const char * sFilename, U16 usPOSX, U16 usPOSY) 
{
	GUI_HMEM hMem;
	char *_acBuffer2;
	int fd;

	/* ����һ���ڴ�ռ� ���ҽ������� */
    hMem = GUI_ALLOC_AllocZero(Picture_Buffer);
	/* �����뵽�ڴ�ľ��ת����ָ������ */
	_acBuffer2 = GUI_ALLOC_h2p(hMem);

	
    /* ���ļ� */		
	fd = open(sFilename, O_RDONLY,0);
	
	if (fd < 0)
	{
		rt_kprintf("��ͼƬʧ�ܣ����\\n");
		return;
	}

	/* ����ͼ�굽��̬�ڴ� */
	 read(fd, _acBuffer2,Picture_Buffer);

	/* ��ʾPNGͼ�� */
	GUI_PNG_Draw(_acBuffer2, Picture_Buffer , usPOSX, usPOSY);
 	GUI_ALLOC_Free(hMem);
 	close(fd);
}


/*
*********************************************************************************************************
*	�� �� ��: _ShowGIF
*	����˵��: ��ʾGIFƬ   ���ص�SRAM����ʾ
*	��    �Σ�sFilename Ҫ��ʾ��ͼƬ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void _ShowGIF_RAM(const char * sFilename) 
{
	int fd;
	GUI_HMEM hMem;
	char *_acBuffer2;
	unsigned char i = 0;

	/* ����һ���ڴ�ռ� ���ҽ������� */
    hMem = GUI_ALLOC_AllocZero(Picture_Buffer);
	/* �����뵽�ڴ�ľ��ת����ָ������ */
	_acBuffer2 = GUI_ALLOC_h2p(hMem);

	
    /* ���ļ� */		
	fd = open(sFilename, O_RDONLY,0);
	
	if (fd < 0)
	{
		rt_kprintf("��ͼƬʧ�ܣ����\\n");
		return;
	}

    read(fd, _acBuffer2, Picture_Buffer);

	
   GUI_GIF_GetInfo(_acBuffer2, Picture_Buffer, &InfoGif1);
   
   while(1)
   {
		if(i < InfoGif1.NumImages)
	    {
	        GUI_GIF_GetImageInfo(_acBuffer2, Picture_Buffer, &InfoGif2, i);
	        GUI_GIF_DrawSub(_acBuffer2, 
			                Picture_Buffer, 
			                (LCD_GetXSize() - InfoGif1.xSize)/2,
							(LCD_GetYSize() - InfoGif1.ySize)/2, 
			                i++);
			
            /* ����ѡ��Ŵ���ʾ */			
// 			GUI_GIF_DrawSubScaled(_acBuffer2, 
// 			                      file.fsize, 
// 			                      (LCD_GetXSize() - InfoGif1.xSize*3)/2,
// 							      (LCD_GetYSize() - InfoGif1.ySize*3)/2, 
// 			                      i++,
// 			                      3,
//                                1);
			
			GUI_X_Delay(InfoGif2.Delay);					  
	    }
	    else
	    {
	        i = 0;
	    }
	}

	/* ʵ��Ӧ���У�����ʵ������ͷŶ�̬�ڴ� */		
 	GUI_ALLOC_Free(hMem);
 	close(fd);
}



/*
*********************************************************************************************************
*	�� �� ��: _ShowJPG
*	����˵��: ��ʾJPEGͼƬ   ���ص�SRAM����ʾ
*	��    �Σ�sFilename Ҫ��ʾ��ͼƬ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void _ShowJPG_RAM(const char * sFilename) 
{
	int fd;
	GUI_HMEM hMem;
	char *_acBuffer2;

	/* ����һ���ڴ�ռ� ���ҽ������� */
    hMem = GUI_ALLOC_AllocZero(Picture_Buffer);
	/* �����뵽�ڴ�ľ��ת����ָ������ */
	_acBuffer2 = GUI_ALLOC_h2p(hMem);

	
    /* ���ļ� */		
	fd = open(sFilename, O_RDONLY,0);
	
	if (fd < 0)
	{
		rt_kprintf("��ͼƬʧ�ܣ����\\n");
		return;
	}

	/* ����ͼƬ���ڴ��� */
   read(fd, _acBuffer2, Picture_Buffer);


	/* ��ʾͼƬ */
	GUI_JPEG_GetInfo(_acBuffer2, Picture_Buffer, &JpegInfo);
	GUI_JPEG_Draw(_acBuffer2,
				  Picture_Buffer, 
			      0, 
			      0);

	GUI_ALLOC_Free(hMem);
	close(fd);
	
}

/*
*********************************************************************************************************
*	�� �� ��: MainTask
*	����˵��: GUI������ 
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Ma(void) 
{	
	GUI_Init();

	/* ����Ƥ������ */
	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
	CHECKBOX_SetDefaultSkin(CHECKBOX_SKIN_FLEX);
	DROPDOWN_SetDefaultSkin(DROPDOWN_SKIN_FLEX);
	SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
	SLIDER_SetDefaultSkin(SLIDER_SKIN_FLEX);
	HEADER_SetDefaultSkin(HEADER_SKIN_FLEX);
	RADIO_SetDefaultSkin(RADIO_SKIN_FLEX);
	
	//_ShowBMP_RAM("SD/1.bmp");	
	
  /* �����Ѽ��ص��洢����PNG�ļ� */
 	_ShowPNG_RAM("SD/1.png", 0, 0);
 	_ShowPNG_RAM("SD/2.png", 100, 0);
	_ShowPNG_RAM("SD/3.png", 200, 0);
 	_ShowPNG_RAM("SD/4.png", 300, 0);

//   _ShowGIF_RAM("/SD/4.gif");	
//	 _ShowJPG_RAM("/SD/2.jpg");


	while(1)
	{
	GUI_SetBkColor(GUI_WHITE);
	GUI_Clear();
	/* ����������ص��洢����PNG�ļ� */
	_ShowPNGEx("/SD/1.png", 0, 100);
		GUI_Delay(200);
	_ShowPNGEx("/SD/2.png", 100,100);
		GUI_Delay(200);
	_ShowPNGEx("/SD/3.png", 200, 100);
		GUI_Delay(200);
	_ShowPNGEx("/SD/4.png", 300,100);
		GUI_Delay(200);
	
  GUI_Delay(200);
	
	GUI_SetBkColor(GUI_WHITE);
	GUI_Clear();
	_ShowBMPEx("/SD/1.bmp",0,0);
	GUI_Delay(200);	
		
//	GUI_SetBkColor(GUI_WHITE);
//	GUI_Clear();
//	_ShowJPGEx("/SD/2.jpg");
//	GUI_Delay(200);
		
	GUI_SetBkColor(GUI_WHITE);
	GUI_Clear();
	//_ShowJPGEx("/SD/11.jpg");
	GUI_Delay(200);
	

	
	GUI_SetBkColor(GUI_WHITE);
	GUI_Clear();
	_ShowGIFEx("/SD/4.gif");
	}
	
	while(1)
	GUI_Delay(200);


}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
