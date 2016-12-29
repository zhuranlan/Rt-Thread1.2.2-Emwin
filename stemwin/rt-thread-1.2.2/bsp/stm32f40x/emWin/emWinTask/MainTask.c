///*
//*********************************************************************************************************
//*	                                  
//*	模块名称 : GUI界面任务
//*	文件名称 : MainTask.c
//*	版    本 : V4.0
//*	说    明 : GUI任务
//*	修改记录 :
//*		版本号    日期          作者               说明
//*		V1.0    2013-03-26    Eric2013        版本emWin5.16
//*       V2.0    2014-02-23    Eric2013        版本STemWin5.22
//*	    V3.0    2015-04-08    Eric2013        1. 升级固件库到V1.5.0
//*                                             2. 升级BSP板级支持包 
//*                                             3. 升级fatfs到0.11
//*                                             4. 升级STemWin到5.26
//*                                             5. 更改为新的四点触摸校准算法
//*                                             6. 添加7寸800*480分辨率电容屏支持，添加3.5寸480*320的ILI9488支持。
//*                                             7. 修改对话框显示大小固定为480*272，以此来适应所有屏。
//*                                             8. 重新整理注释，使其更加规范。
//*	    V4.0    2015-12-18  Eric2013          1. 升级BSP板级支持包 
//*                                             2. 升级STemWin到5.28
//*                                             3. 添加4.3寸和5寸电容屏支持。
//*
//*      Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
//*
//*********************************************************************************************************
//*/
//#include "MainTask.h"
//extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ48;
//extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ24;
//extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ16;
//extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ12;

///*
//*********************************************************************************************************
//*	                                    变量    
//*********************************************************************************************************
//*/
//GUI_COLOR _acColor[3] = {GUI_BLUE,GUI_RED,GUI_YELLOW};                                              
//static int BackColor;                                                                   

///*
//*********************************************************************************************************
//*	                          GUI_WIDGET_CREATE_INFO类型数组 
//*********************************************************************************************************
//*/
//static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
//    { FRAMEWIN_CreateIndirect,  "陕西科技大学",            0,                 0,  0,  480,272,FRAMEWIN_CF_MOVEABLE,0},
//    { BUTTON_CreateIndirect,    "上翻",           GUI_ID_BUTTON0,          52, 93, 110,30, 0,0},
//    { BUTTON_CreateIndirect,    "下翻",           GUI_ID_BUTTON1,          289,90, 117,33, 0,0}
//};

///*
//*********************************************************************************************************
//*	函 数 名: PaintDialog
//*	功能说明: 对话框回调函数重绘消息的执行函数
//*	形    参: pMsg  消息
//*	返 回 值: 无
//*********************************************************************************************************
//*/
//void PaintDialog(WM_MESSAGE * pMsg)                                                               
//{
////    WM_HWIN hWin = pMsg->hWin;
//	GUI_SetBkColor(_acColor[BackColor]);                                                       
//	GUI_Clear();
//}

///*
//*********************************************************************************************************
//*	函 数 名: InitDialog
//*	功能说明: 对话框回调函数初始化消息的执行函数
//*	形    参: pMsg  消息
//*	返 回 值: 无
//*********************************************************************************************************
//*/
//void InitDialog(WM_MESSAGE * pMsg)
//{
//    WM_HWIN hWin = pMsg->hWin;
//    
//	//
//    //FRAMEWIN
//    //
//    FRAMEWIN_SetFont(hWin,&GUI_FontHZ24);
//    FRAMEWIN_SetTextAlign(hWin,GUI_TA_VCENTER|GUI_TA_CENTER);
//    FRAMEWIN_AddCloseButton(hWin, FRAMEWIN_BUTTON_RIGHT, 0);
//    FRAMEWIN_AddMaxButton(hWin, FRAMEWIN_BUTTON_RIGHT, 1);
//    FRAMEWIN_AddMinButton(hWin, FRAMEWIN_BUTTON_RIGHT, 2);
//    FRAMEWIN_SetTitleHeight(hWin,30);
//    
//	//
//    //GUI_ID_BUTTON0
//    //
//    BUTTON_SetFont(WM_GetDialogItem(hWin,GUI_ID_BUTTON0),&GUI_FontHZ12);                      
//    
//	//
//    //GUI_ID_BUTTON1
//    //
//    BUTTON_SetFont(WM_GetDialogItem(hWin,GUI_ID_BUTTON1),&GUI_FontHZ16);

//	/* 默认颜色取*/
//	BackColor = 0;  
//}

///*
//*********************************************************************************************************
//*	函 数 名: _cbCallback
//*	功能说明: 对话框回调函数
//*	形    参: pMsg  消息
//*	返 回 值: 无
//*********************************************************************************************************
//*/
//static void _cbCallback(WM_MESSAGE * pMsg) 
//{
//    int NCode, Id;
//    WM_HWIN hWin = pMsg->hWin;
//    switch (pMsg->MsgId) 
//    {
//        case WM_PAINT:
//            PaintDialog(pMsg);
//            break;
//        case WM_INIT_DIALOG:
//            InitDialog(pMsg);
//            break;
//        case WM_KEY:
//            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
//            {
//                case GUI_KEY_ESCAPE:
//                    GUI_EndDialog(hWin, 1);
//                    break;
//                case GUI_KEY_ENTER:
//                    GUI_EndDialog(hWin, 0);
//                    break;
//            }
//            break;
//        case WM_NOTIFY_PARENT:
//            Id = WM_GetId(pMsg->hWinSrc); 
//            NCode = pMsg->Data.v;        
//            switch (Id) 
//            {
//                case GUI_ID_OK:
//                    if(NCode==WM_NOTIFICATION_RELEASED)
//                        GUI_EndDialog(hWin, 0);
//                    break;
//                case GUI_ID_CANCEL:
//                    if(NCode==WM_NOTIFICATION_RELEASED)
//                        GUI_EndDialog(hWin, 0);
//                    break;
//                case GUI_ID_BUTTON0:
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

//            }
//            break;
//        default:
//            WM_DefaultProc(pMsg);
//    }
//}

///*
//*********************************************************************************************************
//*	函 数 名: MainTask
//*	功能说明: GUI主函数
//*	形    参: 无
//*	返 回 值: 无
//*********************************************************************************************************
//*/
//void MainTask(void) 
//{ 
//    GUI_Init();
//    WM_SetDesktopColor(GUI_WHITE);      
//    WM_SetCreateFlags(WM_CF_MEMDEV);   
//	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
//	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
//	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
//	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
//	CHECKBOX_SetDefaultSkin(CHECKBOX_SKIN_FLEX);
//	DROPDOWN_SetDefaultSkin(DROPDOWN_SKIN_FLEX);
//	SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
//	SLIDER_SetDefaultSkin(SLIDER_SKIN_FLEX);
//	HEADER_SetDefaultSkin(HEADER_SKIN_FLEX);
//	RADIO_SetDefaultSkin(RADIO_SKIN_FLEX);
//    GUI_ExecDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);           
//}

///***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/


/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.22                          *
*        Compiled Jul  4 2013, 15:16:01                              *
*        (c) 2013 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/

// USER START (Optionally insert additional includes)
// USER END




//#include "GUI.h"

////在触摸校正的时候使用这个来读取ADC的值

//void Touch_MainTask(void) {
// 
//  GUI_CURSOR_Show();
//  GUI_CURSOR_Select(&GUI_CursorCrossL);
//  GUI_SetBkColor(GUI_WHITE);
//  GUI_SetColor(GUI_BLACK);
//  GUI_Clear();
//  GUI_DispString("Measurement of\nA/D converter values");
//  while (1) {
//    GUI_PID_STATE TouchState;
//    int xPhys, yPhys;
//    GUI_TOUCH_GetState(&TouchState);  /* Get the touch position in pixel */
//    xPhys = GUI_TOUCH_GetxPhys();     /* Get the A/D mesurement result in x */
//    yPhys = GUI_TOUCH_GetyPhys();     /* Get the A/D mesurement result in y */
//    /* Display the measurement result */
//    GUI_SetColor(GUI_BLUE);
//    GUI_DispStringAt("Analog input:\n", 0, 20);
//    GUI_GotoY(GUI_GetDispPosY() + 2);
//    GUI_DispString("x:");
//    GUI_DispDec(xPhys, 4);
//    GUI_DispString(", y:");
//    GUI_DispDec(yPhys, 4);
//    /* Display the according position */
//    GUI_SetColor(GUI_RED);
//    GUI_GotoY(GUI_GetDispPosY() + 4);
//    GUI_DispString("\nPosition:\n");
//    GUI_GotoY(GUI_GetDispPosY() + 2);
//    GUI_DispString("x:");
//    GUI_DispDec(TouchState.x,4);
//    GUI_DispString(", y:");
//    GUI_DispDec(TouchState.y,4);
//    /* Wait a while */
//    GUI_Delay(10);
//  };
//}

