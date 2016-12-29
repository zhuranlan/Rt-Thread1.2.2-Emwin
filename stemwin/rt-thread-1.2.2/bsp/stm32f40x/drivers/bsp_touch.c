#include "rtthread.h"
#include "stm32f4xx_conf.h"
#include "GUI.h"
#include "bsp_touch.h"

#define ADC_CH_X	1		/* X通道，测量X位置 */
#define ADC_CH_Y	5		/* Y通道，测量Y位置 */

/* 每1ms扫描一次坐标 */
#define DOWN_VALID		30	/* 按下30ms 后, 开始统计ADC */
#define SAMPLE_COUNT	10	/* 按下后连续采集40个样本 */

/*
触摸屏校准点相对屏幕像素四角的偏移像素
第1个点 ： x1 = CALIB_OFFSET, y1 = CALIB_OFFSET
第2个点 ： x2 = LCD_GetWidth() - CALIB_OFFSET, y2 = LCD_GetHeight() - CALIB_OFFSET
*/
#define CALIB_OFFSET	20
#define TP_X1	CALIB_OFFSET
#define TP_Y1	CALIB_OFFSET

#define TP_X2	(LCD_GetWidth() - CALIB_OFFSET)
#define TP_Y2	(LCD_GetHeight() - CALIB_OFFSET)

#define TP_X3	CALIB_OFFSET
#define TP_Y3	(LCD_GetHeight() - CALIB_OFFSET)

#define TP_X4	(LCD_GetWidth() - CALIB_OFFSET)
#define TP_Y4	CALIB_OFFSET

/* 有效ADC值的判断门限. 太接近ADC临界值的坐标认为无效 */
#define ADC_VALID_OFFSET	2

//#define WaitTPReady() while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5))
#define WaitTPReady() {}

/* 触屏模块用到的全局变量 */
TOUCH_T g_tTP;

static uint8_t	TOUCH_PressValid(uint16_t _usX, uint16_t _usY);
static uint16_t TOUCH_DataFilter(uint16_t *_pBuf, uint8_t _ucCount);
static void TOUCH_LoadParam(void);
static void TOUCH_SaveParam(void);
static int32_t CalTwoPoint(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x);
static int16_t TOUCH_TransX(uint16_t _usAdcX, uint16_t _usAdcY);
static int16_t TOUCH_TransY(uint16_t _usAdcX, uint16_t _usAdcY);
int32_t TOUCH_Abs(int32_t x);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitTouch
*	功能说明: 配置STM32和触摸相关的口线，使能硬件SPI1, 片选由软件控制
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_InitHard(void)
{
	uint16_t i;
	
    g_tTP.Enable = 0;
    g_tTP.Write = g_tTP.Read = 0;	

	{	
		TSC2046_InitHard();
		
		g_tTP.usMaxAdc = 4095;	/* 12位ADC */
	}

	g_tTP.Enable = 1;

}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_ReadAdcX
*	功能说明: 获得触摸板X方向ADC采样值， 已进行滤波处理
*	形    参:  无
*	返 回 值: X 方向ADC值
*********************************************************************************************************
*/
uint16_t TOUCH_ReadAdcX(void)
{
	uint16_t usAdc;

	__set_PRIMASK(1);  		/* 关中断 */
	usAdc = g_tTP.usAdcNowX;
	__set_PRIMASK(0);  		/* 开中断 */

	return usAdc;
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_ReadAdcY
*	功能说明: 获得触摸板Y方向ADC采样值， 已进行滤波处理
*	形    参:  无
*	返 回 值: Y 坐标值，允许负值
*********************************************************************************************************
*/
uint16_t TOUCH_ReadAdcY(void)
{
	uint16_t usAdc;

	__set_PRIMASK(1);  		/* 关中断 */
	usAdc = g_tTP.usAdcNowY;
	__set_PRIMASK(0);  		/* 开中断 */

	return usAdc;
}


/*
*********************************************************************************************************
*	函 数 名: TOUCH_GetKey
*	功能说明: 从触摸FIFO缓冲区读取一个坐标值。
*	形    参:  无
*	返 回 值:
*			TOUCH_NONE      表示无事件
*			TOUCH_DOWN      按下
*			TOUCH_MOVE      移动
*			TOUCH_RELEASE	释放
*********************************************************************************************************
*/
uint8_t TOUCH_GetKey(int16_t *_pX, int16_t *_pY)
{
	uint8_t ret;

	if (g_tTP.Read == g_tTP.Write)
	{
		return TOUCH_NONE;
	}
	else
	{
		ret = g_tTP.Event[g_tTP.Read];
		*_pX = g_tTP.XBuf[g_tTP.Read];
		*_pY = g_tTP.YBuf[g_tTP.Read];

		if (++g_tTP.Read >= TOUCH_FIFO_SIZE)
		{
			g_tTP.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_CelarFIFO
*	功能说明: 清除触摸FIFO缓冲区
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_CelarFIFO(void)
{
	__set_PRIMASK(1);  		/* 关中断 */
	g_tTP.Write = g_tTP.Read;
	__set_PRIMASK(0);  		/* 开中断 */
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_InRect
*	功能说明: 判断当前坐标是否位于矩形框内
*	形    参:  _usX, _usY: 输入坐标
*			_usRectX,_usRectY: 矩形起点
*			_usRectH、_usRectW : 矩形高度和宽度
*	返 回 值: 1 表示在范围内
*********************************************************************************************************
*/
uint8_t TOUCH_InRect(uint16_t _usX, uint16_t _usY,
	uint16_t _usRectX, uint16_t _usRectY, uint16_t _usRectH, uint16_t _usRectW)
{
	if ((_usX > _usRectX) && (_usX < _usRectX + _usRectW)
		&& (_usY > _usRectY) && (_usY < _usRectY + _usRectH))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_MoveValid
*	功能说明: 判断当前坐标和上次坐标是否偏差太大
*	形    参:  _usX1, _usY1: 坐标1
*			  _usX2, _usY2: 坐标2
*	返 回 值: 1 表示有效点， 0 表示飞点
*********************************************************************************************************
*/
uint8_t TOUCH_MoveValid(uint16_t _usX1, uint16_t _usY1, uint16_t _usX2, uint16_t _usY2)
{
	int16_t iX, iY;
	static uint8_t s_invalid_count = 0;

	iX = TOUCH_Abs(_usX1 - _usX2);
	iY = TOUCH_Abs(_usY1 - _usY2);

	if ((iX < 25) && (iY < 25))
	{
		s_invalid_count = 0;
		return 1;
	}
	else
	{
		if (++s_invalid_count >= 3)
		{
			s_invalid_count = 0;
			return 1;
		}
		return 0;
	}
}



/*
*********************************************************************************************************
*	函 数 名: TOUCH_Scan
*	功能说明: 触摸板事件检测程序。该函数被周期性调用，每ms调用1次. 见 bsp_Timer.c
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
GUI_PID_STATE State;	
uint16_t usAdcX;
uint16_t usAdcY;
void TOUCH_Scan(void)
{

	static uint16_t s_usXBuf[SAMPLE_COUNT];
	static uint16_t s_usYBuf[SAMPLE_COUNT];
	static uint8_t s_ucPos = 0;
	static uint8_t s_count = 0;
	static uint8_t s_down = 0;
	static uint16_t s_usSaveAdcX, s_usSaveAdcY; /* 用于触笔抬起事件，保存按下和移动的最后采样值 */

	if (g_tTP.Enable == 0)
	{
		return;
	}


		if (TSC2046_PenInt())
		{

			usAdcX = TSC2046_ReadAdc(ADC_CH_X);
			usAdcY = TSC2046_ReadAdc(ADC_CH_Y);
//			usAdcX=(3780-usAdcX)+250;
//			usAdcY=(3780-usAdcY)+150;
//		rt_kprintf("usAdcX:%d \r\n",usAdcX);
//		rt_kprintf("usAdcY:%d \r\n",usAdcY);
	
		}
		else
		{
			usAdcX = 0;
			usAdcY = 0;
		}
//	

//	if (TOUCH_PressValid(usAdcX, usAdcY))
//	{
//		/* 按压30ms之后才开始采集数据 */
//		if (s_count >= DOWN_VALID)
//		{
//			s_usXBuf[s_ucPos] = usAdcX;
//			s_usYBuf[s_ucPos] = usAdcY;

//			/* 采集10ms数据进行滤波 */
//			if (++s_ucPos >= SAMPLE_COUNT)
//			{
//				s_ucPos = 0;
//				/* 对ADC采样值进行软件滤波 */
//				g_tTP.usAdcNowX = TOUCH_DataFilter(s_usXBuf, SAMPLE_COUNT);
//				g_tTP.usAdcNowY = TOUCH_DataFilter(s_usYBuf, SAMPLE_COUNT);

//				if (s_down == 0)
//				{
//					s_down = 1;

//					/* 触摸按下事件 */
//					State.x = TOUCH_TransX( g_tTP.usAdcNowX, g_tTP.usAdcNowY);
//					State.y = TOUCH_TransY( g_tTP.usAdcNowX, g_tTP.usAdcNowY);
//					State.Pressed = 1;
//					GUI_PID_StoreState(&State);

//					/* 用于触笔抬起事件，保存按下和移动的最后采样值 */
//					s_usSaveAdcX = g_tTP.usAdcNowX;
//					s_usSaveAdcY = g_tTP.usAdcNowY;
//					

//				}
//				else
//				{
//					if (TOUCH_MoveValid(s_usSaveAdcX, s_usSaveAdcY, g_tTP.usAdcNowX, g_tTP.usAdcNowY))
//					{
//						/* 触摸移动事件 */
//						State.x = TOUCH_TransX( g_tTP.usAdcNowX, g_tTP.usAdcNowY);
//						State.y = TOUCH_TransY( g_tTP.usAdcNowX, g_tTP.usAdcNowY);
//						State.Pressed = 1;
//						GUI_PID_StoreState(&State);

//						/* 用于触笔抬起事件，保存按下和移动的最后采样值 */
//						s_usSaveAdcX = g_tTP.usAdcNowX;
//						s_usSaveAdcY = g_tTP.usAdcNowY;
//					}
//					else
//					{
//						g_tTP.usAdcNowX = 0; /* for debug stop */
//					}
//				}

//			}
//		}
//		else
//		{
//			s_count++;
//		}
//	}
//	else
//	{
//		if (s_count > 0)
//		{
//			if (--s_count == 0)
//			{
//				/* 触摸释放事件 */
//				/* State.x和State.y的数值无需更新，State是全局变量，保存的就是最近一次的数值 */
//				State.Pressed = 0;
//				GUI_PID_StoreState(&State);
//				s_count = 0;
//				s_down = 0;

//				g_tTP.usAdcNowX = 0;
//				g_tTP.usAdcNowY = 0;
//			}
//		}
//		s_ucPos = 0;
//	}
}

/*
*********************************************************************************************************
*	函 数 名: CalTwoPoint
*	功能说明: 根据2点直线方程，计算Y值
*	形    参:  2个点的坐标和x输入量
*	返 回 值: x对应的y值
*********************************************************************************************************
*/
static int32_t CalTwoPoint(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x)
{
	return y1 + ((int32_t)(y2 - y1) * (x - x1)) / (x2 - x1);
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_TransX
*	功能说明: 将触摸ADC值转换为像素坐标
*	形    参:  无
*	返 回 值: X 坐标值，允许负值
*********************************************************************************************************
*/
static int16_t TOUCH_TransX(uint16_t _usAdcX, uint16_t _usAdcY)
{
#if CALIB_POINT_COUNT == 2	/* 2点校准 */
	uint16_t x;
	int32_t y;

	if (g_tTP.XYChange == 0)
	{
		x = _usAdcX;
		if (x == 0)
		{
			y = 0;
		}
		else
		{
			//y = CalTwoPoint(g_tTP.usAdcX1, TP_X1, g_tTP.usAdcX2, TP_X2, x);
			y = CalTwoPoint(g_tTP.usAdcX1, g_tTP.usLcdX1, g_tTP.usAdcX2, g_tTP.usLcdX2, x);
		}
	}
	else
	{
		x = _usAdcY;
		if (x == 0)
		{
			y = 0;
		}
		else
		{
			//y = CalTwoPoint(g_tTP.usAdcY1, TP_X1, g_tTP.usAdcY2, TP_X2, x);
			y = CalTwoPoint(g_tTP.usAdcY1, g_tTP.usLcdX1, g_tTP.usAdcY2, g_tTP.usLcdX2, x);
		}
	}
	return y;
#else		/* 4点校准 */
	uint16_t x, x1, x2;
	int32_t y;

	if (g_tTP.XYChange == 0)	/* X Y 坐标不交换 */
	{
		x = _usAdcX;

		/* 根据 Y ADC 实时计算直线方程的参考点x1, x2
			if  _usAdcY = usAdcY1 then  取点 = (AdcX1, TP_X1, AdcX4, TP_X4, _usAdcY)
			if  _usAdcY = usAdcY2 then  取点 = (AdcX3, TP_X3, AdcX2, TP_X2, _usAdcY)

			其中 TP_X1 = TP_X3;  TP_X4 = TP_X1 , 这是程序设定的校准位置的像素坐标, 是固定的。
			我们仅需要动态计算对第1个和第3个参数。同样采用2点直线方程计算。
		*/
		x1 = CalTwoPoint(g_tTP.usAdcY1, g_tTP.usAdcX1, g_tTP.usAdcY2,  g_tTP.usAdcX3, _usAdcY);
		x2 = CalTwoPoint(g_tTP.usAdcY1, g_tTP.usAdcX4, g_tTP.usAdcY2,  g_tTP.usAdcX2, _usAdcY);
	}
	else						/* X Y 坐标交换 */
	{
		x = _usAdcY;

		/* 根据 X ADC 实时计算直线方程的参考点x1, x2
			if  _usAdcX = usAdcX1 then  取点 = (AdcY1, TP_X1, AdcY4, TP_X4, _usAdcX)
			if  _usAdcX = usAdcX2 then  取点 = (AdcY3, TP_X3, AdcY2, TP_X2, _usAdcX)

			其中 TP_X1 = TP_X3;  TP_X4 = TP_X1 , 这是程序设定的校准位置的像素坐标, 是固定的。
			我们仅需要动态计算对第1个和第3个参数。同样采用2点直线方程计算。
		*/
		x1 = CalTwoPoint(g_tTP.usAdcX1, g_tTP.usAdcY1, g_tTP.usAdcX2,  g_tTP.usAdcY3, _usAdcX);
		x2 = CalTwoPoint(g_tTP.usAdcX1, g_tTP.usAdcY4, g_tTP.usAdcX2,  g_tTP.usAdcY2, _usAdcX);
	}

	if (x == 0)
	{
		y = 0;
	}
	else
	{
		/* 根据2点直线方程，计算坐标 */
		y = CalTwoPoint(x1, TP_X1, x2, TP_X2, x);
		//CalTwoPoint(x1, g_tTP.usLcdX1, x2, g_tTP.usLcd2, x);
	}
	return y;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_TransY
*	功能说明: 将触摸ADC值转换为像素坐标
*	形    参:  无
*	返 回 值: Y 坐标值，允许负值
*********************************************************************************************************
*/
static int16_t TOUCH_TransY(uint16_t _usAdcX, uint16_t _usAdcY)
{
#if CALIB_POINT_COUNT == 2	/* 2点校准 */
	int32_t x;
	int32_t y;

	if (g_tTP.XYChange == 0)
	{
		x = _usAdcY;
		if (x == 0)
		{
			y = 0;
		}
		else
		{
			//y = CalTwoPoint(g_tTP.usAdcY1, TP_Y1, g_tTP.usAdcY2, TP_Y2, x);
			y = CalTwoPoint(g_tTP.usAdcY1, g_tTP.usLcdY1, g_tTP.usAdcY2, g_tTP.usLcdY2, x);
		}
	}
	else
	{
		x = _usAdcX;
		if (x == 0)
		{
			y = 0;
		}
		else
		{
			//y = CalTwoPoint(g_tTP.usAdcX1, TP_Y1, g_tTP.usAdcX2, TP_Y2, x);
			y = CalTwoPoint(g_tTP.usAdcX1, g_tTP.usLcdY1, g_tTP.usAdcX2, g_tTP.usLcdY2, x);
		}
	}
	return y;
#else		/* 4点校准 */
	int32_t x, x1, x2;
	int32_t y;

	if (g_tTP.XYChange == 0)	/* X Y 坐标不交换 */
	{
		x = _usAdcY;

		/* 根据 X ADC 实时计算直线方程的参考点x1, x2
			if  _usAdcX = usAdcX1 then  取点 = (AdcY1, TP_Y1, AdcY3, TP_Y3, _usAdcX)
			if  _usAdcX = usAdcX2 then  取点 = (AdcY4, TP_Y4, AdcY2, TP_Y2, _usAdcX)

			其中 TP_Y1 = TP_Y4;  TP_Y3 = TP_Y2 , 这是程序设定的校准位置的像素坐标, 是固定的。
			我们仅需要动态计算对第1个和第3个参数。同样采用2点直线方程计算。
		*/
		x1 = CalTwoPoint(g_tTP.usAdcX1, g_tTP.usAdcY1, g_tTP.usAdcX2,  g_tTP.usAdcY4, _usAdcX);
		x2 = CalTwoPoint(g_tTP.usAdcX1, g_tTP.usAdcY3, g_tTP.usAdcX2,  g_tTP.usAdcY2, _usAdcX);
	}
	else						/* X Y 坐标交换 */
	{
		x = _usAdcX;

		/* 根据 X ADC 实时计算直线方程的参考点x1, x2
			if  _usAdcY = usAdcY1 then  取点 = (AdcX1, TP_Y1, AdcX3, TP_Y3, _usAdcY)
			if  _usAdcY = usAdcY2 then  取点 = (AdcX4, TP_Y4, AdcX2, TP_Y2, _usAdcY)

			其中 TP_Y1 = TP_Y3;  TP_Y4 = TP_Y2 , 这是程序设定的校准位置的像素坐标, 是固定的。
			我们仅需要动态计算对第1个和第3个参数。同样采用2点直线方程计算。
		*/
		x1 = CalTwoPoint(g_tTP.usAdcY1, g_tTP.usAdcX1, g_tTP.usAdcY2,  g_tTP.usAdcX4, _usAdcY);
		x2 = CalTwoPoint(g_tTP.usAdcY1, g_tTP.usAdcX3, g_tTP.usAdcY2,  g_tTP.usAdcX2, _usAdcY);
	}

	if (x == 0)
	{
		y = 0;
	}
	else
	{
		/* 根据2点直线方程，计算坐标 */
		y = CalTwoPoint(x1, TP_Y1, x2, TP_Y2, x);
		//y = CalTwoPoint(x1, g_tTP.usLcdY1, x2, g_tTP.usLcdY2, x);
	}
	return y;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_GetX
*	功能说明: 获得当前的的触摸坐标X
*	形    参:  无
*	返 回 值: X 坐标值，允许负值
*********************************************************************************************************
*/
int16_t TOUCH_GetX(void)
{
	return TOUCH_TransX(TOUCH_ReadAdcX(), TOUCH_ReadAdcY());
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_GetY
*	功能说明: 获得当前的的触摸坐标Y
*	形    参:  无
*	返 回 值: Y 坐标值，允许负值
*********************************************************************************************************
*/
int16_t TOUCH_GetY(void)
{
	return TOUCH_TransY(TOUCH_ReadAdcX(), TOUCH_ReadAdcY());
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_DataFilter
*	功能说明: 对采样数据进行滤波
*	形    参:  无
*	返 回 值: X 坐标值，允许负值
*********************************************************************************************************
*/
static uint16_t TOUCH_DataFilter(uint16_t *_pBuf, uint8_t _ucCount)
{
#if 0
	uint8_t i;
	uint32_t uiSum;

	uiSum = 0;
	for (i = 0; i < _ucCount; i++)
	{
		uiSum += _pBuf[i];
	}
	return uiSum / _ucCount;
#else
	uint8_t flag;
	uint8_t i;
	uint16_t usTemp;
	uint32_t uiSum;

	/* 升序排列 */
    do
	{
		flag = 0;
		for (i = 0; i < _ucCount - 1; i++)
		{
			if (_pBuf[i] > _pBuf[i+1])
			{
				usTemp = _pBuf[i + 1];
				_pBuf[i+1] = _pBuf[i];
				_pBuf[i] = usTemp;
				flag = 1;
			}
		}
	}while(flag);

	uiSum = 0;
	for (i = 0; i < _ucCount / 3; i++)
	{
		uiSum += _pBuf[_ucCount / 3 + i];
	}
	usTemp = uiSum / (_ucCount / 3);
	return usTemp;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_DispPoint1
*	功能说明: 显示第1个校准点
*	形    参:  _ucIndex = 0 : 表示第1个点； _ucIndex = 1 表示第2个点;
*	返 回 值: 无
*********************************************************************************************************
*/
//static void TOUCH_DispPoint(uint8_t _ucIndex)
//{
//	FONT_T tFont16;			/* 定义一个字体结构体变量，用于设置字体参数 */

//	/* 设置字体参数 */
//	{
//		tFont16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
//		tFont16.FrontColor = CL_WHITE;		/* 字体颜色 0 或 1 */
//		tFont16.BackColor = CL_BLUE;	/* 文字背景颜色 */
//		tFont16.Space = 0;			/* 文字间距，单位 = 像素 */
//	}

///*
//	第1个点 ： x1 = CALIB_OFFSET, y1 = CALIB_OFFSET
//	第2个点 ： x2 = LCD_GetHeight() - CALIB_OFFSET, y2 = LCD_GetWidth - CALIB_OFFSET
//*/
//	if (_ucIndex == 0)
//	{
//		LCD_ClrScr(CL_BLUE);  		/* 清屏，背景蓝色 */

//		/* 在屏幕边沿绘制2个矩形框(用于检测面板边缘像素是否正常) */
//		LCD_DrawRect(0, 0, LCD_GetHeight(), LCD_GetWidth(), CL_WHITE);
//		LCD_DrawRect(2, 2, LCD_GetHeight() - 4, LCD_GetWidth() - 4, CL_YELLOW);
//		
//		LCD_DispStr(50, 10, "触摸屏四点校准，十秒内不校准将自动返回主界面", &tFont16);		/* 在(8,3)坐标处显示一串汉字 */
//		LCD_DispStr(50, 27, "电阻屏需要校准，电容屏无需校准", &tFont16);

//		LCD_DrawCircle(TP_X1, TP_Y1, 6, CL_WHITE);
//	}
//	else if (_ucIndex == 1)
//	{
//		LCD_DrawCircle(TP_X1, TP_Y1, 6, CL_BLUE);			/* 擦除第1个点 */

//		LCD_DrawCircle(TP_X2, TP_Y2, 6, CL_WHITE);
//	}
//	else if (_ucIndex == 2)
//	{
//		LCD_DrawCircle(TP_X2, TP_Y2, 6, CL_BLUE);			/* 擦除第2个点 */

//		LCD_DrawCircle(TP_X3, TP_Y3, 6, CL_WHITE);
//	}
//	else
//	{
//		LCD_DrawCircle(TP_X3, TP_Y3, 6, CL_BLUE);			/* 擦除第3个点 */

//		LCD_DrawCircle(TP_X4, TP_Y4, 6, CL_WHITE);
//	}
//}


/*
*********************************************************************************************************
*	函 数 名: TOUCH_PressValid
*	功能说明: 判断按压是否有效，根据X, Y的ADC值进行大致判断
*	形    参:  无
*	返 回 值: 1 表示有效； 0 表示无效
*********************************************************************************************************
*/
static uint8_t	TOUCH_PressValid(uint16_t _usX, uint16_t _usY)
{
	if ((_usX <= ADC_VALID_OFFSET) || (_usY <= ADC_VALID_OFFSET)
		|| (_usX >= g_tTP.usMaxAdc - ADC_VALID_OFFSET)
		|| (_usY >= g_tTP.usMaxAdc - ADC_VALID_OFFSET))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_WaitRelease
*	功能说明: 等待触笔释放
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void TOUCH_WaitRelease(void)
{
	uint8_t usCount = 0;

	for (;;)
	{
		if (TOUCH_PressValid(TOUCH_ReadAdcX(), TOUCH_ReadAdcY()) == 0)
		{
			if (++usCount > 5)
			{
				break;
			}
		}
		else
		{
			usCount = 0;
		}
		//bsp_DelayMS(10);
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_Abs
*	功能说明: 计算绝对值
*	形    参: x : 有符合整数
*	返 回 值: 正整数
*********************************************************************************************************
*/
int32_t TOUCH_Abs(int32_t x)
{
	if (x >= 0)
	{
		return x;
	}
	else
	{
		return -x;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_Calibration
*	功能说明: 触摸屏校准
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_Calibration(void)
{
	uint16_t usAdcX;
	uint16_t usAdcY;
	uint8_t usCount;
	uint8_t i;
	uint32_t n;

	TOUCH_CelarFIFO();		/* 清除无效的触摸事件 */

	for (i = 0; i < CALIB_POINT_COUNT; i++)
	{
		//TOUCH_DispPoint(i);		/* 显示校准点 */

		TOUCH_WaitRelease(); 	/* 等待触笔释放 */

		usCount = 0;
		for (n = 0; n < 500; n++)
		{
			usAdcX = TOUCH_ReadAdcX();
			usAdcY = TOUCH_ReadAdcY();

			if (TOUCH_PressValid(usAdcX, usAdcY))
			{
				if (++usCount > 5)
				{
					/* 按压有效, 保存校准点ADC采样值 */
					if (i == 0)
					{
						g_tTP.usAdcX1 = usAdcX;
						g_tTP.usAdcY1 = usAdcY;
					}
					else if (i == 1)
					{
						g_tTP.usAdcX2 = usAdcX;
						g_tTP.usAdcY2 = usAdcY;
					}
					else if (i == 2)
					{
						g_tTP.usAdcX3 = usAdcX;
						g_tTP.usAdcY3 = usAdcY;
					}
					else
					{
						g_tTP.usAdcX4 = usAdcX;
						g_tTP.usAdcY4 = usAdcY;
					}
					break;
				}
			}
			else
			{
				usCount = 0;
			}
			//bsp_DelayMS(10);
		}
		if (n == 500)
		{
			return;
		}
	}

	TOUCH_WaitRelease(); 	/* 等待触笔释放 */

	/* 识别触摸的 X, Y 和 显示面板的 X，Y 是否需要交换 */
	g_tTP.XYChange = 0;		/* 1表示X Y需要交换 */
	if (LCD_GetHeight() < LCD_GetWidth())
	{
		if (TOUCH_Abs(g_tTP.usAdcX1 - g_tTP.usAdcX2) < TOUCH_Abs(g_tTP.usAdcY1 - g_tTP.usAdcY2))
		{
			g_tTP.XYChange = 1;
		}
	}
	else
	{
		if (TOUCH_Abs(g_tTP.usAdcX1 - g_tTP.usAdcX2) > TOUCH_Abs(g_tTP.usAdcY1 - g_tTP.usAdcY2))
		{
			g_tTP.XYChange = 1;
		}
	}

	g_tTP.usLcdX1 = TP_X1;
	g_tTP.usLcdY1 = TP_Y1;
	g_tTP.usLcdX2 = TP_X2;
	g_tTP.usLcdY2 = TP_Y2;
	g_tTP.usLcdX3 = TP_X3;
	g_tTP.usLcdY3 = TP_Y3;
	g_tTP.usLcdX4 = TP_X3;
	g_tTP.usLcdY4 = TP_Y3;

	/* 在最后一步，可以将校准参数保存入Flash 或者EEPROM */
	TOUCH_SaveParam();
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_SaveParam
*	功能说明: 保存校准参数	s_usAdcX1 s_usAdcX2 s_usAdcY1 s_usAdcX2
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
extern void SaveParam(void);
static void TOUCH_SaveParam(void)
{
#if 0
	g_tParam.usAdcX1 = g_tTP.usAdcX1;
	g_tParam.usAdcY1 = g_tTP.usAdcY1;
	g_tParam.usAdcX2 = g_tTP.usAdcX2;
	g_tParam.usAdcY2 = g_tTP.usAdcY2;
	g_tParam.usAdcX3 = g_tTP.usAdcX3;
	g_tParam.usAdcY3 = g_tTP.usAdcY3;
	g_tParam.usAdcX4 = g_tTP.usAdcX4;
	g_tParam.usAdcY4 = g_tTP.usAdcY4;

	g_tParam.usLcdX1 = g_tTP.usLcdX1;
	g_tParam.usLcdY1 = g_tTP.usLcdY1;
	g_tParam.usLcdX2 = g_tTP.usLcdX2;
	g_tParam.usLcdY2 = g_tTP.usLcdY2;
	g_tParam.usLcdX3 = g_tTP.usLcdX3;
	g_tParam.usLcdY3 = g_tTP.usLcdY3;
	g_tParam.usLcdX4 = g_tTP.usLcdX4;
	g_tParam.usLcdY4 = g_tTP.usLcdY4;

	g_tParam.XYChange = g_tTP.XYChange;

	g_tParam.TouchDirection = g_LcdDirection;	/* 2014-09-11 添加屏幕方向, 用于屏幕旋转时无需再次校准 */

	SaveParam();	/* 将参数写入Flash */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_LoadParam
*	功能说明: 读取校准参数
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
extern void LoadParam(void);
static void TOUCH_LoadParam(void)
{
#if 0
	LoadParam();	/* 从Flash中读取参数 */

	g_tTP.usAdcX1 = g_tParam.usAdcX1;
	g_tTP.usAdcY1 = g_tParam.usAdcY1;
	g_tTP.usAdcX2 = g_tParam.usAdcX2;
	g_tTP.usAdcY2 = g_tParam.usAdcY2;
	g_tTP.usAdcX3 = g_tParam.usAdcX3;
	g_tTP.usAdcY3 = g_tParam.usAdcY3;
	g_tTP.usAdcX4 = g_tParam.usAdcX4;
	g_tTP.usAdcY4 = g_tParam.usAdcY4;

	g_tTP.usLcdX1 = g_tParam.usLcdX1;
	g_tTP.usLcdY1 = g_tParam.usLcdY1;
	g_tTP.usLcdX2 = g_tParam.usLcdX2;
	g_tTP.usLcdY2 = g_tParam.usLcdY2;
	g_tTP.usLcdX3 = g_tParam.usLcdX3;
	g_tTP.usLcdY3 = g_tParam.usLcdY3;
	g_tTP.usLcdX4 = g_tParam.usLcdX4;
	g_tTP.usLcdY4 = g_tParam.usLcdY4;

	g_tTP.XYChange = g_tParam.XYChange;
#endif

	g_tTP.usAdcX1 = 3791;
	g_tTP.usAdcY1 = 266;
	g_tTP.usAdcX2 = 242;
	g_tTP.usAdcY2 = 288;
	g_tTP.usAdcX3 = 255;
	g_tTP.usAdcY3 = 3708;
	g_tTP.usAdcX4 = 3784;
	g_tTP.usAdcY4 = 3749;
	
	g_tTP.usLcdX1 = 0;
	g_tTP.usLcdY1 = 0;
	g_tTP.usLcdX2 = 480;
	g_tTP.usLcdY2 = 0;
	g_tTP.usLcdX3 = 480;
	g_tTP.usLcdY3 = 320;
	g_tTP.usLcdX4 = 0;
	g_tTP.usLcdY4 = 320;


	
//	g_tTP.XYChange = g_tParam.XYChange;

}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/