/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.28                          *
*        Compiled Jan 30 2015, 16:41:06                              *
*        (c) 2015 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/

// USER START (Optionally insert additional includes)
// USER END

#include "DIALOG.h"


/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_FRAMEWIN_0    (GUI_ID_USER + 0x00)
#define ID_MULTIPAGE_0    (GUI_ID_USER + 0x04)
#define ID_GRAPH_0        (GUI_ID_USER + 0x05)
#define ID_WINDOW_0    (GUI_ID_USER + 0x06)


// USER START (Optionally insert additional defines)
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ48;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ24;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ16;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ12;
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

// USER START (Optionally insert additional static data)
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "System", ID_FRAMEWIN_0, 0, 0, 480, 320, 0, 0x0, 0 },
 // { MULTIPAGE_CreateIndirect, "Multipage", ID_MULTIPAGE_0, 0, 0, 470, 270, 0, 0x0, 0 },
	 // { GRAPH_CreateIndirect, "Graph", ID_GRAPH_0, 42, 68, 337, 158, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

static const GUI_WIDGET_CREATE_INFO _aDialogCreate1[] = {
  { WINDOW_CreateIndirect,    "Dialog 1", 0,                   0,   0, 260, 100, FRAMEWIN_CF_MOVEABLE },
  { BUTTON_CreateIndirect,    "Button",   GUI_ID_BUTTON0,      5,  30,  80,  20, 0},
  { TEXT_CreateIndirect,      "Dialog 1", 0,                   5,  10,  50,  20, TEXT_CF_LEFT }
};

static const GUI_WIDGET_CREATE_INFO _aDialogCreate2[] = {
  { WINDOW_CreateIndirect,   NULL, ID_WINDOW_0,                   0,   0, 480, 270, FRAMEWIN_CF_MOVEABLE },
	{ GRAPH_CreateIndirect, "Graph", ID_GRAPH_0, 0, 40, 480, 240, 0 },
};

static const GUI_WIDGET_CREATE_INFO _aDialogCreate3[] = {
   { WINDOW_CreateIndirect,    NULL, 0,                   0,   0, 480, 320, FRAMEWIN_CF_MOVEABLE },
  { TEXT_CreateIndirect, "MCU: STM32F407IGT6", GUI_ID_TEXT0, 11, 10, 191, 20,  TEXT_CF_LEFT },
  { TEXT_CreateIndirect, "RAM: 192K+512K", GUI_ID_TEXT1, 11, 30, 182, 20,  TEXT_CF_LEFT },
  { TEXT_CreateIndirect, "Clock: 235M", GUI_ID_TEXT2, 11, 50, 184, 20, TEXT_CF_LEFT },
	{ TEXT_CreateIndirect, "EOS: RT-Thread 1.2.2", GUI_ID_TEXT3, 11, 70, 191, 20,  TEXT_CF_LEFT },
	{ TEXT_CreateIndirect, "Gui: Stemwin 5.28", GUI_ID_TEXT4, 11, 90, 191, 20,  TEXT_CF_LEFT },
  { TEXT_CreateIndirect, "FPGA: CYCLONE II", GUI_ID_TEXT5, 237, 10, 214, 20,  TEXT_CF_LEFT},
  { TEXT_CreateIndirect, "LEs: 8256", GUI_ID_TEXT6, 237, 30, 128, 30,  TEXT_CF_LEFT },
  { TEXT_CreateIndirect, "User I/Os: 208", GUI_ID_TEXT7, 237, 50, 214, 20,  TEXT_CF_LEFT},
  { TEXT_CreateIndirect, "Memory Bits: 165888", GUI_ID_TEXT8, 237, 70, 214, 20, TEXT_CF_LEFT },
};
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
		  //
    //FRAMEWIN
    //
    FRAMEWIN_SetFont(hWin,&GUI_Font24B_ASCII);
    FRAMEWIN_SetTextAlign(hWin,GUI_TA_VCENTER|GUI_TA_CENTER);
    FRAMEWIN_AddCloseButton(hWin, FRAMEWIN_BUTTON_RIGHT, 0);
    FRAMEWIN_AddMaxButton(hWin, FRAMEWIN_BUTTON_RIGHT, 1);
    FRAMEWIN_AddMinButton(hWin, FRAMEWIN_BUTTON_RIGHT, 2);
    FRAMEWIN_SetTitleHeight(hWin,25);
}

unsigned int current_id,before_id;
static void _update(WM_MESSAGE * pMsg)
{
	  WM_HWIN hItem;
    WM_HWIN hWin = pMsg->hWin;
	 // hItem = WM_GetDialogItem(pMsg->hWin, ID_WINDOW_0);	
	  hItem = WM_GetDialogItem(hWin, ID_MULTIPAGE_0);	
	  current_id=MULTIPAGE_GetSelection(hItem);
	  rt_kprintf("move here %d \n",current_id);
	  if(current_id!=before_id)
		{
	  rt_kprintf("move here %d \n",current_id);
			switch(current_id)
			{
				case 0: break;
				case 1:  break;
				case 2: break;
			}
		}
	  before_id=current_id;
}


/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

// USER START (Optionally insert additional static code)
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
	WM_HWIN hItem;
  int     NCode;
  int     Id;
  // USER START (Optionally insert additional variables)
  // USER END
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Multipage'
    //
    InitDialog(pMsg);
    // USER START (Optionally insert additional code for further widget initialization)
    // USER END
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_MULTIPAGE_0: // Notifications sent by 'Multipage'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_MOVED_OUT:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
_update(pMsg);
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    // USER START (Optionally insert additional code for further Ids)
    // USER END
    }
    break;
  // USER START (Optionally insert additional message handling)
  // USER END
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}


static void _cbDialogCreate2(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
  // USER START (Optionally insert additional variables)
  // USER END

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Multipage'
    //

    // USER START (Optionally insert additional code for further widget initialization)
    // USER END
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_MULTIPAGE_0: // Notifications sent by 'Multipage'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_MOVED_OUT:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
          _update(pMsg);
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    // USER START (Optionally insert additional code for further Ids)
    // USER END
    }
    break;
  // USER START (Optionally insert additional message handling)
  // USER END
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}



/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CreateFramewin
*/
WM_HWIN System_source(void);
WM_HWIN System_source(void) {
  WM_HWIN hWin;
	WM_HWIN hDialog;

  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, 0, 0, 0); 
	//
  // Create and attach the MULTIPAGE dialog windows
  //
  hWin=MULTIPAGE_CreateEx(0, 0, 470, 270, WM_GetClientWindow(hWin), WM_CF_SHOW, 0, ID_MULTIPAGE_0);

	MULTIPAGE_SetFont(hWin,&GUI_FontHZ16);
  hDialog = GUI_CreateDialogBox(_aDialogCreate1, GUI_COUNTOF(_aDialogCreate1), 0,       hWin, 0, 0);
  MULTIPAGE_AddPage(hWin, hDialog, "任务");
  hDialog = GUI_CreateDialogBox(_aDialogCreate2, GUI_COUNTOF(_aDialogCreate2), 0,       hWin, 0, 0);
  MULTIPAGE_AddPage(hWin, hDialog, "资源");
  hDialog = GUI_CreateDialogBox(_aDialogCreate3, GUI_COUNTOF(_aDialogCreate3), 0,       hWin, 0, 0);
  MULTIPAGE_AddPage(hWin, hDialog, "属性");
	MULTIPAGE_SelectPage(hWin,1);
	
	
	hDialog = WM_GetDialogItem(hWin, ID_GRAPH_0);	
	GRAPH_SetGridVis  (hDialog, 1);
  GRAPH_SetGridDistX(hDialog, 25);
  GRAPH_SetGridDistY(hDialog, 10);
  WM_BringToBottom  (hDialog);
	
  return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/