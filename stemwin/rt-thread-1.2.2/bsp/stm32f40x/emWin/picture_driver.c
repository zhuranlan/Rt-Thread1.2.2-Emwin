

#include "picture_driver.h"

//	static char _acBuffer[0x2000];
	static char _acBuffer[20];
	#define Picture_Buffer 100*1024
	GUI_JPEG_INFO JpegInfo;
	GUI_GIF_INFO InfoGif1;
	GUI_GIF_IMAGE_INFO InfoGif2;

/*
*********************************************************************************************************
*	函 数 名: _GetData
*	功能说明: 被函数GUI_BMP_DrawEx()调用
*	形    参：p             FIL类型数据
*             NumBytesReq   请求读取的字节数
*             ppData        数据指针
*             Off           如果Off = 1，那么将重新从其实位置读取                 
*	返 回 值: 返回读取的字节数
*********************************************************************************************************
*/
static int _GetData(int  fd_p,const U8 ** ppData, unsigned NumBytesReq, U32 Off) 
{
	U32 i;
	
	static int FileAddress = 0;

	/*
	* 检测缓存大小
	*/
	if (NumBytesReq > sizeof(_acBuffer)) {
	NumBytesReq = sizeof(_acBuffer);
	}

	/*
	* 设置读取位置
	*/
	if(Off == 1) FileAddress = 0;
	else FileAddress = Off;
	
  lseek(fd_p, FileAddress,SEEK_SET);

	
	/*
	* 读取数据到缓存, 由于FatFS+官方SD卡的方案存在DMA传输上的4字节对齐问题，这里以小于等于一个
	* SD卡扇区大小来操作，超过512字节会出错。
	*/
	for(i = 0; i < NumBytesReq / 512; i++)
	{
		read(fd_p, &_acBuffer[512*i], 512);
	}
	
	 read(fd_p, &_acBuffer[512*i], NumBytesReq % 512);


	/*
	* 让指针ppData指向读取的函数
	*/
	*ppData = (const U8 *)&_acBuffer[0];

	/*
	* 返回读取的字节数
	*/
	return NumBytesReq;
}


/*
*********************************************************************************************************
*	函 数 名: _GetData PNG
*	功能说明: 被函数GUI_PNG_DrawEx调用
*	形    参: p             FIL类型数据
*             NumBytesReq   请求读取的字节数
*             ppData        数据指针
*             Off           如果Off = 1，那么将重新从其实位置读取                 
*	返 回 值: 返回读取的字节数
*********************************************************************************************************
*/
static int _GetData_PNG(int fd_p, const U8 ** ppData, unsigned NumBytesReq, U32 Off) 
{
	static int FileAddress = 0;


	U8     * pData;

	pData  = (U8 *)*ppData;

	
	//
	// 设置数据读取位置
	//
	if(Off == 1) FileAddress = 0;
	else FileAddress = Off;
	lseek(fd_p, FileAddress,SEEK_SET);
	//
	// 读取数据到缓存
	//
 read(fd_p, pData, NumBytesReq);

	//
	// 返回读取大小
	//
	return NumBytesReq;

}

/*
*********************************************************************************************************
*	函 数 名: _ShowBMPEx
*	功能说明: 显示BMP图片
*	形    参: sFilename 要显示图片的名字
*	返 回 值: 无
*********************************************************************************************************
*/
void _ShowBMPEx(const char * sFilename, U16 usPOSX, U16 usPOSY) 
{	

   int fd;
	
    /* 打开文件 */		
	fd = open(sFilename, O_RDONLY,0);
	
	if (fd < 0)
	{
		rt_kprintf("打开图片失败！！\n");
		return;
	}
	
//	XSize = GUI_BMP_GetXSizeEx(_GetData, &file);
//	YSize = GUI_BMP_GetYSizeEx(_GetData, &file);


	  GUI_BMP_DrawEx(_GetData,(int *)fd, usPOSX, usPOSY);
	
	  close(fd);
}




/*
*********************************************************************************************************
*	函 数 名: _ShowJPGEx
*	功能说明: 显示JPEG图片
*	形    参：sFilename 要显示图片的名字
*	返 回 值: 无
*********************************************************************************************************
*/
 void _ShowJPGEx(const char * sFilename, U16 usPOSX, U16 usPOSY) 
{	

	unsigned int i;
	int fd;

	/* 打开文件 */		
	fd = open(sFilename,  O_RDONLY,0);
	if (fd < 0)
	{
		rt_kprintf("打开图片失败！！\n");
		return;
	}
	
	GUI_JPEG_GetInfoEx(_GetData, (int *)fd, &JpegInfo);
	//while(1)
	{
		/**********必要的时候，可以加上调度锁，防止刷图片的时候死机*************/
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
*	函 数 名: _ShowGIFEx
*	功能说明: GIF图片显示
*	形    参: sFilename 文件名
*	返 回 值: 无
*********************************************************************************************************
*/
 void _ShowGIFEx(const char * sFilename) 
{	

	unsigned int i,j;
	int fd;

	/* 打开文件 */		
	  fd=open( sFilename,  O_RDONLY,0);
	if (fd < 0)
	{
		rt_kprintf("打开图片失败！！\n");
		return;
	}

	/* 获取GIF图片信息 */
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
*	函 数 名: _ShowPNG
*	功能说明: 显示PNG图片
*	形    参: sFilename 要显示的图片名字
*             usPOSX    显示位置X
*             usPOSY    显示位置Y
*	返 回 值: 无
*********************************************************************************************************
*/
 void _ShowPNGEx(const char * sFilename, U16 usPOSX, U16 usPOSY) 
{	
int fd;
	/* 打开文件 */				
	fd=open( sFilename,  O_RDONLY,0);
	if (fd < 0)
	{
		rt_kprintf("打开图片失败！！\n");
		return;
	}
	
	GUI_PNG_DrawEx(_GetData_PNG, (int *)fd, usPOSX, usPOSY);

	close(fd);
}



/*
*********************************************************************************************************
*	函 数 名: _ShowBMP
*	功能说明: 显示BMP图片  加载到SRAM中显示
*	形    参: sFilename 要显示的图片名字
*	返 回 值: 无
*********************************************************************************************************
*/
void _ShowBMP_RAM(const char * sFilename) 
{
	int XSize, YSize;
	GUI_HMEM hMem;
	char *_acBuffer2;
	int i;
	int fd;

	/* 申请一块内存空间 并且将其清零 */
    hMem = GUI_ALLOC_AllocZero(Picture_Buffer);
	/* 将申请到内存的句柄转换成指针类型 */
	_acBuffer2 = GUI_ALLOC_h2p(hMem);

	
    /* 打开文件 */		
	fd = open(sFilename, O_RDONLY,0);
	
	if (fd < 0)
	{
		rt_kprintf("打开图片失败！！\n");
		return;
	}
	
	/* 读取图片到缓冲中 */
	 read(fd,_acBuffer2, Picture_Buffer);


	XSize = GUI_BMP_GetXSize(_acBuffer2);
	YSize = GUI_BMP_GetYSize(_acBuffer2);

	while(1)
	{
		for(i = 100; i < 600; i += 10)
		{
			/* 放缩显示 */
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
*	函 数 名: _ShowPNG
*	功能说明: 显示GIF片   加载到SRAM中显示
*	形    参：sFilename 要显示的图片名字
*             usPOSX    显示位置X
*             usPOSY    显示位置Y
*	返 回 值: 无
*********************************************************************************************************
*/
 void _ShowPNG_RAM(const char * sFilename, U16 usPOSX, U16 usPOSY) 
{
	GUI_HMEM hMem;
	char *_acBuffer2;
	int fd;

	/* 申请一块内存空间 并且将其清零 */
    hMem = GUI_ALLOC_AllocZero(Picture_Buffer);
	/* 将申请到内存的句柄转换成指针类型 */
	_acBuffer2 = GUI_ALLOC_h2p(hMem);

	
    /* 打开文件 */		
	fd = open(sFilename, O_RDONLY,0);
	
	if (fd < 0)
	{
		rt_kprintf("打开图片失败！！\n");
		return;
	}

	/* 加载图标到动态内存 */
	 read(fd, _acBuffer2,Picture_Buffer);

	/* 显示PNG图标 */
	GUI_PNG_Draw(_acBuffer2, Picture_Buffer , usPOSX, usPOSY);
 	GUI_ALLOC_Free(hMem);
 	close(fd);
}


/*
*********************************************************************************************************
*	函 数 名: _ShowGIF
*	功能说明: 显示GIF片   加载到SRAM中显示
*	形    参：sFilename 要显示的图片名字
*	返 回 值: 无
*********************************************************************************************************
*/
void _ShowGIF_RAM(const char * sFilename) 
{
	int fd;
	GUI_HMEM hMem;
	char *_acBuffer2;
	unsigned char i = 0;

	/* 申请一块内存空间 并且将其清零 */
    hMem = GUI_ALLOC_AllocZero(Picture_Buffer);
	/* 将申请到内存的句柄转换成指针类型 */
	_acBuffer2 = GUI_ALLOC_h2p(hMem);

	
    /* 打开文件 */		
	fd = open(sFilename, O_RDONLY,0);
	
	if (fd < 0)
	{
		rt_kprintf("打开图片失败！！\n");
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
			
            /* 可以选择放大显示 */			
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

	/* 实际应用中，根据实际情况释放动态内存 */		
 	GUI_ALLOC_Free(hMem);
 	close(fd);
}



/*
*********************************************************************************************************
*	函 数 名: _ShowJPG
*	功能说明: 显示JPEG图片   加载到SRAM中显示
*	形    参：sFilename 要显示的图片名字
*	返 回 值: 无
*********************************************************************************************************
*/
void _ShowJPG_RAM(const char * sFilename) 
{
	int fd;
	GUI_HMEM hMem;
	char *_acBuffer2;

	/* 申请一块内存空间 并且将其清零 */
    hMem = GUI_ALLOC_AllocZero(Picture_Buffer);
	/* 将申请到内存的句柄转换成指针类型 */
	_acBuffer2 = GUI_ALLOC_h2p(hMem);

	
    /* 打开文件 */		
	fd = open(sFilename, O_RDONLY,0);
	
	if (fd < 0)
	{
		rt_kprintf("打开图片失败！！\n");
		return;
	}

	/* 加载图片到内存中 */
   read(fd, _acBuffer2, Picture_Buffer);


	/* 显示图片 */
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
*	函 数 名: MainTask
*	功能说明: GUI主函数 
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void Ma(void) 
{	
	GUI_Init();

	/* 设置皮肤函数 */
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
	
  /* 绘制已加载到存储器的PNG文件 */
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
	/* 绘制无需加载到存储器的PNG文件 */
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
