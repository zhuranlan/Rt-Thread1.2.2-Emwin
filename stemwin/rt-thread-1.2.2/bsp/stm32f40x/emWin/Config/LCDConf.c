/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2015  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.28 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
----------------------------------------------------------------------
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUIDRV_Template.h"
#include "stm32f4xx.h"
#include "GUIDRV_FlexColor.h"

/*
********************************************************************
*
*       Layer configuration (to be modified)
*
********************************************************************
*/
// Physical display size
//

#define YSIZE_PHYS  480
#define XSIZE_PHYS  320
#define VYSIZE_PHYS 480
#define VXSIZE_PHYS 320
#define LCD_CONTROLLER       8875 
#define LCD_BITSPERPIXEL       16
#define LCD_USE_PARALLEL_16     0
//
// Color conversion
//
#define COLOR_CONVERSION GUICC_M565
//
// Display driver
//
#define DISPLAY_DRIVER  &GUIDRV_Template_API
//
// Orientation
//
#define DISPLAY_ORIENTATION  GUI_SWAP_XY 
//
// Configures touch screen module
//
#define TOUCH_AD_LEFT 	 223
#define TOUCH_AD_RIGHT 	 3769
#define TOUCH_AD_TOP 	   3754
#define TOUCH_AD_BOTTOM  247


/*
**********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif

#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif

#ifndef   VXSIZE_PHYS
  #define VXSIZE_PHYS XSIZE_PHYS
#endif

#ifndef   VYSIZE_PHYS
  #define VYSIZE_PHYS YSIZE_PHYS
#endif

#ifndef   VRAM_ADDR
  #define VRAM_ADDR 0
#endif

#ifndef   COLOR_CONVERSION
  #error Color conversion not defined!
#endif

#ifndef   DISPLAY_DRIVER
  #error No display driver defined!
#endif

	
	
/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/
	
#define LCD_REG_ADDRESS		  *(__IO uint16_t *)(0x68000000)
#define LCD_DATA_ADDRESS		*(__IO uint16_t *)(0x68020000)
	
	
/********************************************************************
*
*       LcdWriteReg
*
* Function description:
*   Sets display register
*/
static void LcdWriteReg(U16 Data) {
  // ... TBD by user
	LCD_REG_ADDRESS=Data;
}

/********************************************************************
*
*       LcdWriteData
*
* Function description:
*   Writes a value to a display register
*/
static void LcdWriteData(U16 Data) {
  // ... TBD by user
	LCD_DATA_ADDRESS=Data;
}

/********************************************************************
*
*       LcdWriteDataMultiple
*
* Function description:
*   Writes multiple values to a display register.
*/
static void LcdWriteDataMultiple(U16 * pData, int NumItems) {
  while (NumItems--) {
    // ... TBD by user
		LCD_DATA_ADDRESS=*pData++;
  }
}

/********************************************************************
*
*       LcdReadDataMultiple
*
* Function description:
*   Reads multiple values from a display register.
*/
static void LcdReadDataMultiple(U16 * pData, int NumItems) {
  *pData = LCD_DATA_ADDRESS;
	while (NumItems--) {
    // ... TBD by user
		*pData++=LCD_DATA_ADDRESS;
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
*       LCD_X_Config
*
* Function description
*   Called during the initialization process in order to set up the
*   display driver configuration.
*/
void LCD_X_Config(void) {
	GUI_DEVICE * pDevice;
	CONFIG_FLEXCOLOR Config = {0};
	GUI_PORT_API PortAPI = {0};
	
//	GUI_MULTIBUF_Config(100*1024);
	
	/* Set display driver and color conversion for 1st layer */
  //GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);
  pDevice = GUI_DEVICE_CreateAndLink(GUIDRV_FLEXCOLOR, GUICC_565, 0, 0);

  LCD_SetSizeEx (0, XSIZE_PHYS , YSIZE_PHYS);
  LCD_SetVSizeEx(0, VXSIZE_PHYS, VYSIZE_PHYS);
	
	GUI_TOUCH_Calibrate(GUI_COORD_X, 0, 320, TOUCH_AD_BOTTOM, TOUCH_AD_TOP);
  GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, 480, TOUCH_AD_RIGHT , TOUCH_AD_LEFT);	
  //
  // Orientation
  //配置显示屏的方向
  Config.Orientation =  DISPLAY_ORIENTATION ;
	GUIDRV_FlexColor_Config(pDevice, &Config);
	//
  // Set controller and operation mode
  //
  PortAPI.pfWrite16_A0  = LcdWriteReg;
  PortAPI.pfWrite16_A1  = LcdWriteData;
  PortAPI.pfWriteM16_A1 = LcdWriteDataMultiple;
  PortAPI.pfReadM16_A1  = LcdReadDataMultiple;

	//配置总线,缓存,和硬件程序  5.22手册991页有详细解说所支持的控制器
  GUIDRV_FlexColor_SetFunc(pDevice, &PortAPI, GUIDRV_FLEXCOLOR_F66709, GUIDRV_FLEXCOLOR_M16C0B16);
}

/*
*********************************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if 
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*******************************************************************************
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) 
{
	int r;
	switch (Cmd) 
	{
		case LCD_X_INITCONTROLLER: 
		{
			//
			// Called during the initialization process in order to set up the
			// display controller and put it into operation. If the display
			// controller is not initialized by any external routine this needs
			// to be adapted by the customer...
			//
			// ...
			//  已经在前面初始化了，这里不再初始化
			
			return 0;
		}
		case LCD_X_SETVRAMADDR: 
		{
			//
			// Required for setting the address of the video RAM for drivers
			// with memory mapped video RAM which is passed in the 'pVRAM' element of p
			//
			//...
			return 0;
		}
		case LCD_X_SETORG: 
		{
			//
			// Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
			//

			//...
			return 0;
		}
		case LCD_X_SHOWBUFFER: 
		{
			//
			// Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
			//
			//...
			return 0;
		}
		case LCD_X_SETLUTENTRY: 
		{
			//
			// Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
			//
			//...
			return 0;
		}
		case LCD_X_ON: 
		{
			//
			// Required if the display controller should support switching on and off
			//
			return 0;
		}
		case LCD_X_OFF: 
		{
			//
			// Required if the display controller should support switching on and off
			//
			// ...
			return 0;
		}
		default:
		r = -1;
	}
	return r;
}

/*************************** End of file ****************************/
