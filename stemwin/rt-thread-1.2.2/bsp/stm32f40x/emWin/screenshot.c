#include "rtthread.h"
#include "stm32f4xx.h"
#include "GUI.h"
#include <dfs.h>
#include <dfs_posix.h>
/**
  *****************************************************************************
  * @Name   : emWin???????????
  *
  * @Brief  : none
  *
  * @Input  : Data: ??????
  *           *p:   ??
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
static void _WriteByte2File(unsigned char Data, int  p)
{
      //  UINT nWrite = 0;
         
      //  f_write((FIL *)p, (TCHAR *)&Data, 1, &nWrite);
//	
	      write(p, &Data, 1);
}
 
/**
  *****************************************************************************
  * @Name   : emWin????????????
  *
  * @Brief  : none
  *
  * @Input  : *FileName: ????
  *           (x0, y0):  ????
  *           xSize:     ??X????
  *           ySize:     ??Y????
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void emWin_CreateBMPPicture(const char * FileName, int x0, int y0, int xSize, int ySize)
{
       // static FIL hFile;
	      int fd;
        GUI_COLOR forecolor, backcolor;
         
        //
        //????,?????
        //
        forecolor = GUI_GetColor();  //??????
        backcolor = GUI_GetBkColor();  //??????
        //
        //??????
        //
        fd=open(FileName, O_WRONLY | O_TRUNC,0);
        GUI_BMP_SerializeEx(_WriteByte2File, x0, y0, xSize, ySize, (int *)fd);
        close(fd);  //??,????
         
        GUI_SetColor(forecolor);  //??????
        GUI_SetBkColor(backcolor);  //??????
}
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(emWin_CreateBMPPicture, screenshot);//emWin_CreateBMPPicture("/SD/A.BMP",0,0,480,320)
#endif
