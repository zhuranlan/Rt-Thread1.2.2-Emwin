#include "GUIDEMO.h"

//static int     _HaltTime;
//static int     _HaltTimeStart;
//static int     _Halt;

///*********************************************************************
//*
//*       GUIDEMO_GetTime
//*/
//int GUIDEMO_GetTime(void) {
//  return _Halt ? _HaltTimeStart : GUI_GetTime() - _HaltTime;
//}


/*********************************************************************
*
*       GUIDEMO_Main
*/
void GUIDEMO_Main(void) {
  FRAMEWIN_SKINFLEX_PROPS Framewin_Props;

  WM_SetCreateFlags(WM_CF_MEMDEV); 
	
  BUTTON_SetReactOnLevel();
  FRAMEWIN_GetSkinFlexProps(&Framewin_Props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
  Framewin_Props.Radius = 0;
  FRAMEWIN_SetSkinFlexProps(&Framewin_Props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
  FRAMEWIN_GetSkinFlexProps(&Framewin_Props, FRAMEWIN_SKINFLEX_PI_INACTIVE);
  Framewin_Props.Radius = 0;
  FRAMEWIN_SetSkinFlexProps(&Framewin_Props, FRAMEWIN_SKINFLEX_PI_INACTIVE);
  PROGBAR_SetDefaultSkin   (PROGBAR_SKIN_FLEX);
  BUTTON_SetDefaultSkin    (BUTTON_SKIN_FLEX);
  SCROLLBAR_SetDefaultSkin (SCROLLBAR_SKIN_FLEX);
  SLIDER_SetDefaultSkin    (SLIDER_SKIN_FLEX);
  HEADER_SetDefaultSkin    (HEADER_SKIN_FLEX);
  GUI_SetTextMode          (GUI_TM_TRANS);
}

/*************************** End of file ****************************/

