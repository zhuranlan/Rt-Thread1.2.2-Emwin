/*
*********************************************************************************************************
*
*	ģ������ : TSC2046���败��оƬ��������
*	�ļ����� : bsp_tsc2046.c
*	��    �� : V1.0
*	˵    �� : TSC2046����оƬ��������
*	�޸ļ�¼ :
*		�汾��   ����        ����     ˵��
*		V1.0    2014-10-23  armfly   ��ʽ����
*
*	Copyright (C), 2014-2015, ���������� www.armfly.com
*********************************************************************************************************
*/

#include "rtthread.h"
#include "stm32f4xx_conf.h"
#define HARD_SPI
void TSC2046_InitHard(void);
uint16_t TSC2046_ReadAdc(uint8_t _ucCh);
uint8_t TSC2046_PenInt(void);

/*
��1��������STM32-V5������ + 3.5����ʾģ�飬 ��ʾģ���ϵĴ���оƬΪ TSC2046�������оƬ
	PI11   --> TP_CS
	PD3/LCD_BUSY   --> TP_BUSY  û��ʹ��
	PI1/SPI2_SCK   --> TP_SCK
	PC2/SPI2_MISO   --> TP_MISO
	PC3/SPI2_MOSI   --> TP_MOSI

	PI3/TP_INT   --> TP_PEN_INT
*/

/* TSC2046 ����оƬ */
#define TSC2046_RCC_CS 		RCC_AHB1Periph_GPIOH
#define TSC2046_PORT_CS		GPIOH
#define TSC2046_PIN_CS		GPIO_Pin_11
#define TSC2046_CS_1()		TSC2046_PORT_CS->BSRRL = TSC2046_PIN_CS
#define TSC2046_CS_0()		TSC2046_PORT_CS->BSRRH = TSC2046_PIN_CS

#define TSC2046_RCC_INT 	RCC_AHB1Periph_GPIOH
#define TSC2046_PORT_INT	GPIOH
#define TSC2046_PIN_INT		GPIO_Pin_8

void TSC2046_CfgSpiHard(void);
static void TSC2046_SetCS(uint8_t _level);

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitTouch
*	����˵��: ����STM32�ʹ�����صĿ��ߣ�Ƭѡ����������. SPI���ߵ�GPIO��bsp_spi_bus.c ��ͳһ���á�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TSC2046_InitHard(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* ��GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(TSC2046_RCC_CS  | TSC2046_RCC_INT, ENABLE);

	/* ���ü����������IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = TSC2046_PIN_CS;
	GPIO_Init(TSC2046_PORT_CS, &GPIO_InitStructure);

	/* ����TP_BUSY Ϊ��������ģʽ(ʵ����CPU��λ���������״̬) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* �������������� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO������ٶ� */

//	GPIO_InitStructure.GPIO_Pin = TSC2046_PIN_BUSY;
//	GPIO_Init(TSC2046_PORT_BUSY, &GPIO_InitStructure);

	/* ���ô����ж�IO */
	GPIO_InitStructure.GPIO_Pin = TSC2046_PIN_INT;
	GPIO_Init(TSC2046_PORT_INT, &GPIO_InitStructure);

	TSC2046_CfgSpiHard();
}

/*
*********************************************************************************************************
*	�� �� ��: TSC2046_ReadAdc
*	����˵��: ѡ��һ��ģ��ͨ��������ADC��������ADC�������
*	��    ��:  _ucCh = 0 ��ʾXͨ���� 1��ʾYͨ��
*	�� �� ֵ: 12λADCֵ
*********************************************************************************************************
*/
uint16_t TSC2046_ReadAdc(uint8_t _ucCh)
{
	uint16_t usAdc;

	TSC2046_SetCS(0);		/* ʹ��TS2046��Ƭѡ */

	/*
		TSC2046 �����֣�8Bit��
		Bit7   = S     ��ʼλ��������1
		Bit6:4 = A2-A0 ģ������ͨ��ѡ��A2-A0; ����6��ͨ����
		Bit3   = MODE  ADCλ��ѡ��0 ��ʾ12Bit;1��ʾ8Bit
		Bit2   = SER/DFR ģ��������ʽ��  1��ʾ�������룻0��ʾ�������
		Bit1:0 = PD1-PD0 ����ģʽѡ��λ
	*/
	bsp_spiWrite0((1 << 7) | (_ucCh << 4));			/* ѡ��ͨ��1, ����Xλ�� */

	/* ��ADC���, 12λADCֵ�ĸ�λ�ȴ���ǰ12bit��Ч�����4bit��0 */
	usAdc = bsp_spiRead0();		/* ���͵�0x00����Ϊ����ֵ�������� */
	usAdc <<= 8;
	usAdc += bsp_spiRead0();		/* ���12λ��ADC����ֵ */

	usAdc >>= 3;						/* ����3λ������12λ��Ч����.  */

	TSC2046_SetCS(1);					/* ����Ƭѡ */

	return (usAdc);
}

/*
*********************************************************************************************************
*	�� �� ��: TSC2046_CfgSpiHard
*	����˵��: ����STM32�ڲ�SPIӲ���Ĺ���ģʽ���ٶȵȲ��������ڷ���TSC2046��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TSC2046_CfgSpiHard(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* ���� SPI1����ģʽ */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 		/* ��������Ƭѡ */
	/*
		SPI_BaudRatePrescaler_64 ��ӦSCKʱ��Ƶ��Լ1M
		TSC2046 ��SCKʱ�ӵ�Ҫ�󣬸ߵ�ƽ�͵͵�ƽ��С200ns������400ns��Ҳ����2.5M

		ʾ����ʵ��Ƶ��
		SPI_BaudRatePrescaler_64 ʱ��SCKʱ��Ƶ��Լ 1.116M
		SPI_BaudRatePrescaler_32 ʱ��SCKʱ��Ƶ���� 2.232M
	*/
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2,&SPI_InitStructure);

	/* ʹ�� SPI2 */
	SPI_Cmd(SPI2,ENABLE);
}

/*
*********************************************************************************************************
*	�� �� ��: TSC2046_SetCS(0)
*	����˵��: ����CS�� ����������SPI������
*	��    ��: ��
	�� �� ֵ: ��
*********************************************************************************************************
*/
static void TSC2046_SetCS(uint8_t _level)
{
	if (_level == 0)
	{
		bsp_SpiBusEnter();	/* ռ��SPI���ߣ� �������߹��� */

		#ifdef SOFT_SPI		/* ����SPI */
			bsp_SetSpiSck(0);
			TSC2046_CS_0();
		#endif

		#ifdef HARD_SPI		/* Ӳ��SPI */
			TSC2046_CS_0();

			bsp_SPI_Init(SPI_Direction_2Lines_FullDuplex | SPI_Mode_Master | SPI_DataSize_8b
				| SPI_CPOL_Low | SPI_CPHA_1Edge | SPI_NSS_Soft | SPI_BaudRatePrescaler_64 | SPI_FirstBit_MSB);
		#endif
	}
	else
	{
		TSC2046_CS_1();

		bsp_SpiBusExit();	/* �ͷ�SPI���ߣ� �������߹��� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: TSC2046_PenInt
*	����˵��: �жϴ������ж�
*	��    ��: ��
*	�� �� ֵ: 0��ʾ���ʰ���  1��ʾ�ͷ�
*********************************************************************************************************
*/
uint8_t TSC2046_PenInt(void)
{
	if (GPIO_ReadInputDataBit(TSC2046_PORT_INT, TSC2046_PIN_INT) == Bit_RESET)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/