/*
*********************************************************************************************************
*	                                  
*	模块名称 : 汉字显示接口
*	文件名称 : GUICharPEx.c
*	版    本 : V1.0
*	说    明 : 这种汉字显示方式适合使用等宽字体，非等宽的微软雅黑等字体显示效果较差。
*              支持点阵：
*                1. 支持12点阵，16点阵，24点阵以及48点阵汉字。
*                2. 6X12点阵的ASCII，8X16点阵的ASCII，12X24点阵的ASCII，24X48点阵的ASCII。
*              字库存储方式：
*                1. 字库存储到SD里面。
*
*	修改记录 :
*		版本号    日期           作者         说明
*		V1.0    2015-04-24     Eric2013       首版
*
*      Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "MainTask.h"
#include "rtthread.h"
#include <dfs_posix.h>



/* 字模数据的暂存数组, 以单个字模的最大字节数为设定值 */ 
#define BYTES_PER_FONT      1024*2 
static U8 GUI_FontDataBuf[BYTES_PER_FONT];

/* 
   用于12*12和16*16电子汉字的路径部分，要和其它汉字路径作区分 
   使用sizeof的时候，得指定数组的大小。
*/
const char FontEx_HZ[23] = {"/SD/system/gui_font/HZ1"};
 

/*
*********************************************************************************************************
*	函 数 名: GUI_X_GetFontData
*	功能说明: 读取点阵数据
*	形    参: font  字体路径
*             oft   字体的偏移地址
*             ptr   从外部存储器读出字体后，字体存放的缓冲地址。
*             bytes 读出字节数
*	返 回 值: 无
*********************************************************************************************************
*/
void GUI_X_GetFontData(char* font, U32 oft, U8 *ptr, U16 bytes) 
{ 
	  int fd;
    fd = open(font, O_RDONLY ,0);   //打开字库文件   
    if(fd < 0)   
    {   
			rt_kprintf("字库打开失败 \n");
		/*用户可以在这里测试SD卡中字库文件是否成功打开 */
    } 
	
    lseek(fd,oft,SEEK_SET);              //找到首地址  
    read(fd, ptr, bytes); //读取字库点阵数据 
	  close(fd);                  //关闭字体  
} 

/*
*********************************************************************************************************
*	函 数 名: GUI_GetDataFromMemory
*	功能说明: 读取点阵数据
*	形    参: pProp  GUI_FONT_PROP类型结构
*             c      字符
*	返 回 值: 无
*********************************************************************************************************
*/
static void GUI_GetDataFromMemory(const GUI_FONT_PROP GUI_UNI_PTR *pProp, U16P c) 
{ 
    U16 BytesPerFont; 
    U32 oft; 
	char *font = (char *)pProp->paCharInfo->pData; 

	/* 每个字模的数据字节数 */
    BytesPerFont = GUI_pContext->pAFont->YSize * pProp->paCharInfo->BytesPerLine; 
    if (BytesPerFont > BYTES_PER_FONT)
	{
		BytesPerFont = BYTES_PER_FONT;
	}
	/* 英文字符地址偏移算法 */ 
    if (c < 0x80)                                                                
    { 
		oft = (c-0x20) * BytesPerFont; 
    } 
    else                                                                           
    { 
		if(strncmp(FontEx_HZ, font, sizeof(FontEx_HZ)) == 0)
		{
			/* 中文字符地址偏移算法包括符号 */
			oft = ((((c >> 8)-0xA1)) + ((c & 0xFF)-0xA1) * 94)* BytesPerFont; 
		}
		else
		{
			/* 中文字符地址偏移算法包括符号 */
			oft = ((((c >> 8)-0xA1)) + ((c & 0xFF)-0xB0) * 94)* BytesPerFont;
		}
    }

	GUI_X_GetFontData(font, oft, GUI_FontDataBuf, BytesPerFont); 
} 

/*
*********************************************************************************************************
*	函 数 名: GUIPROP_X_DispChar
*	功能说明: 显示字符
*	形    参: c 显示的字符
*	返 回 值: 无
*********************************************************************************************************
*/
void GUIPROP_X_DispChar(U16P c)  
{ 
    int BytesPerLine; 
    GUI_DRAWMODE DrawMode = GUI_pContext->TextMode; 
    const GUI_FONT_PROP GUI_UNI_PTR *pProp = GUI_pContext->pAFont->p.pProp; 

	/* 确定显示的字符是否在有效范围内 */  
    for (; pProp; pProp = pProp->pNext)                                          
    { 
        if ((c >= pProp->First) && (c <= pProp->Last))break; 
    } 
	
	/* 字符有效，进行显示 */
    if (pProp) 
    { 
        GUI_DRAWMODE OldDrawMode;
        const GUI_CHARINFO GUI_UNI_PTR * pCharInfo = pProp->paCharInfo;
        GUI_GetDataFromMemory(pProp, c);
        BytesPerLine = pCharInfo->BytesPerLine;                
        OldDrawMode  = LCD_SetDrawMode(DrawMode);
        LCD_DrawBitmap(GUI_pContext->DispPosX, GUI_pContext->DispPosY,
                       pCharInfo->XSize, GUI_pContext->pAFont->YSize,
                       GUI_pContext->pAFont->XMag, GUI_pContext->pAFont->YMag,
                       1,                  /* Bits per Pixel */
                       BytesPerLine,
                       &GUI_FontDataBuf[0],
                       &LCD_BKCOLORINDEX
                       );
		
        /* 填充背景 */
        if (GUI_pContext->pAFont->YDist > GUI_pContext->pAFont->YSize) 
        {
            int YMag = GUI_pContext->pAFont->YMag;
            int YDist = GUI_pContext->pAFont->YDist * YMag;
            int YSize = GUI_pContext->pAFont->YSize * YMag;
            if (DrawMode != LCD_DRAWMODE_TRANS) 
            {
                LCD_COLOR OldColor = GUI_GetColor();
                GUI_SetColor(GUI_GetBkColor());
                LCD_FillRect(GUI_pContext->DispPosX, GUI_pContext->DispPosY + YSize, 
                             GUI_pContext->DispPosX + pCharInfo->XSize, 
                             GUI_pContext->DispPosY + YDist);
                GUI_SetColor(OldColor);
            }
        }
        LCD_SetDrawMode(OldDrawMode);  /* Restore draw mode */
//      if (!GUI_MoveRTL)
        GUI_pContext->DispPosX += pCharInfo->XDist * GUI_pContext->pAFont->XMag;
    }
} 

/*
*********************************************************************************************************
*	函 数 名: GUIPROP_X_GetCharDistX
*	功能说明: 获取字符的X轴间距
*	形    参: c  字符
*	返 回 值: 无
*********************************************************************************************************
*/
int GUIPROP_X_GetCharDistX(U16P c) 
{
    const GUI_FONT_PROP GUI_UNI_PTR * pProp = GUI_pContext->pAFont->p.pProp;  
    for (; pProp; pProp = pProp->pNext)                                         
    {
        if ((c >= pProp->First) && (c <= pProp->Last))break;
    }
    return (pProp) ? (pProp->paCharInfo)->XSize * GUI_pContext->pAFont->XMag : 0;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

