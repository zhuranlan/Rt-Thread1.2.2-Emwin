  #include "GUI.h"
  #include "WM.h"
  #include "CHECKBOX.h"
  #include "FRAMEWIN.h"
  #include "PROGBAR.h"
  #include "TEXT.h"
  #include "BUTTON.h"
  #include "SLIDER.h"
  #include "HEADER.h"
  #include "GRAPH.h"
  #include "ICONVIEW.h"
  #include "LISTVIEW.h"
  #include "TREEVIEW.h"
	#include "DIALOG.h"

/*
*********************************************************************************************************
*	                                    变量    
*********************************************************************************************************
*/
GUI_COLOR _acColor[3] = {GUI_BLUE,GUI_RED,GUI_YELLOW};                                              
static int BackColor;         

#define ID_FRAMEWIN_0    (GUI_ID_USER + 0x00)
#define ID_TEXT_0    (GUI_ID_USER + 0x01)
#define ID_TEXT_1    (GUI_ID_USER + 0x02)
#define ID_TEXT_2    (GUI_ID_USER + 0x03)
#define ID_TEXT_3    (GUI_ID_USER + 0x04)
#define ID_TEXT_4    (GUI_ID_USER + 0x05)
#define ID_TEXT_5    (GUI_ID_USER + 0x06)
#define ID_TEXT_6    (GUI_ID_USER + 0x07)
#define ID_TEXT_7    (GUI_ID_USER + 0x08)
#define ID_TEXT_8    (GUI_ID_USER + 0x09)


extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ48;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ24;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ16;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ12;

/*
*********************************************************************************************************
*	                          GUI_WIDGET_CREATE_INFO类型数组 
*********************************************************************************************************
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "SYSTEM", 0, -2, 3, 480, 272, FRAMEWIN_CF_MOVEABLE, 0x0, 0 }, 
  { TEXT_CreateIndirect, "MCU: STM32F407IGT6", ID_TEXT_0, 11, 10, 191, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "RAM: 192K+512K", ID_TEXT_1, 11, 30, 182, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Clock: 168M", ID_TEXT_2, 11, 50, 184, 20, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "EOS: RT-Thread 1.2.2", ID_TEXT_7, 11, 70, 191, 20, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "Gui: Stemwin 5.28", ID_TEXT_8, 11, 90, 191, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "FPGA: CYCLONE II", ID_TEXT_3, 237, 10, 214, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "LEs: 8256", ID_TEXT_4, 237, 30, 128, 30, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "User I/Os: 208", ID_TEXT_5, 237, 50, 214, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Memory Bits: 165888", ID_TEXT_6, 237, 70, 214, 20, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

/*
*********************************************************************************************************
*	函 数 名: PaintDialog
*	功能说明: 对话框回调函数重绘消息的执行函数
*	形    参: pMsg  消息
*	返 回 值: 无
*********************************************************************************************************
*/
void PaintDialog(WM_MESSAGE * pMsg)                                                               
{
  WM_HWIN hWin = pMsg->hWin;
	
//	_ShowPNGEx("/SD/ARM.png", 20,100);
//	_ShowPNGEx("/SD/FPGA.png", 250,100);
//_ShowBMPEx("/SD/1.bmp",0,0);
                                                      
}

/*
*********************************************************************************************************
*	函 数 名: InitDialog
*	功能说明: 对话框回调函数初始化消息的执行函数
*	形    参: pMsg  消息
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitDialog(WM_MESSAGE * pMsg)
{	
	   WM_HWIN hItem;
    WM_HWIN hWin = pMsg->hWin;

  	GUI_SetBkColor(GUI_WHITE);  

	  //
    //FRAMEWIN
    //
    FRAMEWIN_SetFont(hWin,&GUI_Font24B_ASCII);
    FRAMEWIN_SetTextAlign(hWin,GUI_TA_VCENTER|GUI_TA_CENTER);
    FRAMEWIN_AddCloseButton(hWin, FRAMEWIN_BUTTON_RIGHT, 0);
    FRAMEWIN_AddMaxButton(hWin, FRAMEWIN_BUTTON_RIGHT, 1);
    FRAMEWIN_AddMinButton(hWin, FRAMEWIN_BUTTON_RIGHT, 2);
    FRAMEWIN_SetTitleHeight(hWin,25);

	  //
    // Initialization of 'MCU:STM32F407IGT6'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
    TEXT_SetFont(hItem, GUI_FONT_20_ASCII);
    //
    // Initialization of 'RAM:192K+512K'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
    TEXT_SetFont(hItem, GUI_FONT_20_ASCII);
    //
    // Initialization of 'F:168M'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
    TEXT_SetFont(hItem, GUI_FONT_20_ASCII);
    //
    // Initialization of 'FPGA:CYCLONE II'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_3);
    TEXT_SetFont(hItem, GUI_FONT_20_ASCII);
    //
    // Initialization of 'RAM:72K'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_4);
    TEXT_SetFont(hItem, GUI_FONT_20_ASCII);
    //
    // Initialization of 'IO:206'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_5);
    TEXT_SetFont(hItem, GUI_FONT_20_ASCII);
		
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_6);
    TEXT_SetFont(hItem, GUI_FONT_20_ASCII);
		
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_7);
    TEXT_SetFont(hItem, GUI_FONT_20_ASCII);
		
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_8);
    TEXT_SetFont(hItem, GUI_FONT_20_ASCII);
    // USER START (Optionally insert additional code for further widget initialization)
//	_ShowPNGEx("/SD/ARM.png", 20,130);
//	_ShowPNGEx("/SD/FPGA.png", 250,130);
    // USER END
    

}

/*
*********************************************************************************************************
*	函 数 名: _cbCallback
*	功能说明: 对话框回调函数
*	形    参: pMsg  消息
*	返 回 值: 无
*********************************************************************************************************
*/
void _cbCallback_System(WM_MESSAGE * pMsg) 
{
    int NCode, Id;

    WM_HWIN hWin = pMsg->hWin;
    switch (pMsg->MsgId) 
    {
        case WM_PAINT:
            PaintDialog(pMsg);
            break;
        case WM_INIT_DIALOG:
            InitDialog(pMsg);
            break;
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                case GUI_KEY_ESCAPE:
                    GUI_EndDialog(hWin, 1);
                    break;
                case GUI_KEY_ENTER:
                    GUI_EndDialog(hWin, 0);
                    break;
            }
            break;
        case WM_NOTIFY_PARENT:
            Id = WM_GetId(pMsg->hWinSrc); 
            NCode = pMsg->Data.v;        
            switch (Id) 
            {
                case GUI_ID_OK:
                    if(NCode==WM_NOTIFICATION_RELEASED)
                        GUI_EndDialog(hWin, 0);
                    break;
                case GUI_ID_CANCEL:
                    if(NCode==WM_NOTIFICATION_RELEASED)
                        GUI_EndDialog(hWin, 0);
                    break;          
            }
            break;
						
        default:
            WM_DefaultProc(pMsg);
    }
		

			
			
}

void systemInfo(WM_HWIN hWin)
{
GUI_ExecDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback_System, 0, 0, 0); 
}


// case GUI_ID_BUTTON0:
//                    switch(NCode)
//                    {
//                        case WM_NOTIFICATION_CLICKED:                                            
//                            BackColor++;
//						if (BackColor == 3)
//						{
//							BackColor = 0;
//						}
//						WM_InvalidateWindow(hWin);
//                            break;
//                        case WM_NOTIFICATION_RELEASED:
//                            
//                            break;
//                        case WM_NOTIFICATION_MOVED_OUT:
//                            
//                            break;
//                    }
//                    break;
//                case GUI_ID_BUTTON1:
//                    switch(NCode)
//                    {
//                        case WM_NOTIFICATION_CLICKED:                                              
//                            BackColor--;
//							if (BackColor < 0)
//							{
//								BackColor = 2;
//							}
//							WM_InvalidateWindow(hWin);
//                            break;
//							
//                        case WM_NOTIFICATION_RELEASED:
//                            break;
//						
//                        case WM_NOTIFICATION_MOVED_OUT:
//                            break;
//                    }
//                    break;