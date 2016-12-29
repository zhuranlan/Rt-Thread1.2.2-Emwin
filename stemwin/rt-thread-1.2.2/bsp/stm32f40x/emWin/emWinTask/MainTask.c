///*
//*********************************************************************************************************
//*	                                  
//*	ģ������ : GUI��������
//*	�ļ����� : MainTask.c
//*	��    �� : V4.0
//*	˵    �� : GUI����
//*	�޸ļ�¼ :
//*		�汾��    ����          ����               ˵��
//*		V1.0    2013-03-26    Eric2013        �汾emWin5.16
//*       V2.0    2014-02-23    Eric2013        �汾STemWin5.22
//*	    V3.0    2015-04-08    Eric2013        1. �����̼��⵽V1.5.0
//*                                             2. ����BSP�弶֧�ְ� 
//*                                             3. ����fatfs��0.11
//*                                             4. ����STemWin��5.26
//*                                             5. ����Ϊ�µ��ĵ㴥��У׼�㷨
//*                                             6. ���7��800*480�ֱ��ʵ�����֧�֣����3.5��480*320��ILI9488֧�֡�
//*                                             7. �޸ĶԻ�����ʾ��С�̶�Ϊ480*272���Դ�����Ӧ��������
//*                                             8. ��������ע�ͣ�ʹ����ӹ淶��
//*	    V4.0    2015-12-18  Eric2013          1. ����BSP�弶֧�ְ� 
//*                                             2. ����STemWin��5.28
//*                                             3. ���4.3���5�������֧�֡�
//*
//*      Copyright (C), 2015-2020, ���������� www.armfly.com
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
//*	                                    ����    
//*********************************************************************************************************
//*/
//GUI_COLOR _acColor[3] = {GUI_BLUE,GUI_RED,GUI_YELLOW};                                              
//static int BackColor;                                                                   

///*
//*********************************************************************************************************
//*	                          GUI_WIDGET_CREATE_INFO�������� 
//*********************************************************************************************************
//*/
//static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
//    { FRAMEWIN_CreateIndirect,  "�����Ƽ���ѧ",            0,                 0,  0,  480,272,FRAMEWIN_CF_MOVEABLE,0},
//    { BUTTON_CreateIndirect,    "�Ϸ�",           GUI_ID_BUTTON0,          52, 93, 110,30, 0,0},
//    { BUTTON_CreateIndirect,    "�·�",           GUI_ID_BUTTON1,          289,90, 117,33, 0,0}
//};

///*
//*********************************************************************************************************
//*	�� �� ��: PaintDialog
//*	����˵��: �Ի���ص������ػ���Ϣ��ִ�к���
//*	��    ��: pMsg  ��Ϣ
//*	�� �� ֵ: ��
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
//*	�� �� ��: InitDialog
//*	����˵��: �Ի���ص�������ʼ����Ϣ��ִ�к���
//*	��    ��: pMsg  ��Ϣ
//*	�� �� ֵ: ��
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

//	/* Ĭ����ɫȡ*/
//	BackColor = 0;  
//}

///*
//*********************************************************************************************************
//*	�� �� ��: _cbCallback
//*	����˵��: �Ի���ص�����
//*	��    ��: pMsg  ��Ϣ
//*	�� �� ֵ: ��
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
//*	�� �� ��: MainTask
//*	����˵��: GUI������
//*	��    ��: ��
//*	�� �� ֵ: ��
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

///***************************** ���������� www.armfly.com (END OF FILE) *********************************/


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

////�ڴ���У����ʱ��ʹ���������ȡADC��ֵ

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

