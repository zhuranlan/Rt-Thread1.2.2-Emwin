#include "mylcd.h"

//LCD��Ҫ������
typedef struct  
{										    
	u16 width;		//LCD ���
	u16 height;		//LCD �߶�
	u16 id;				//LCD ID
	u8  dir;			//���������������ƣ�0��������1��������	
	u8	wramcmd;	//��ʼдgramָ��
	u8  setxcmd;	//����x����ָ��
	u8  setycmd;	//����y����ָ��	 
}lcd_dev_t; 	  
//LCD����
lcd_dev_t lcddev;	//����LCD��Ҫ����

//LCD�Ļ�����ɫ�ͱ���ɫ	   
u16 POINT_COLOR=0x0000;	//������ɫ
u16 BACK_COLOR=0xFFFF;  //����ɫ 

struct rt_device _lcd_device;

//д�Ĵ�������
//regval:�Ĵ���ֵ
 void LCD_WR_REG(u16 regval)
{ 
	 *(volatile u16 *) (LCD_CMD)= regval;
}
//дLCD����
//data:Ҫд���ֵ
 void LCD_WR_DATA(u16 data)
{										    	   
	*(volatile u16 *) (LCD_DATA)=data;		 
}
//��LCD����
//����ֵ:������ֵ
u16 LCD_RD_DATA(void)
{										    	   
	return *(volatile u16 *) (LCD_DATA);		 
}					   
//д�Ĵ���
//LCD_Reg:�Ĵ�����ַ
//LCD_RegValue:Ҫд�������
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{	
	*(volatile u16 *) (LCD_CMD) = LCD_Reg;		//д��Ҫд�ļĴ������	 
	*(volatile u16 *) (LCD_DATA) = LCD_RegValue;//д������	    		 
}	   
//���Ĵ���
//LCD_Reg:�Ĵ�����ַ
//����ֵ:����������
u16 LCD_ReadReg(u8 LCD_Reg)
{										   
	LCD_WR_REG(LCD_Reg);		//д��Ҫ���ļĴ������
	delay_us(5);		  
	return LCD_RD_DATA();		//���ض�����ֵ
}   
//��ʼдGRAM
static void LCD_WriteRAM_Prepare(void)
{
 	*(volatile u16 *) (LCD_CMD)=lcddev.wramcmd;	  
}	 
//LCDдGRAM
//RGB_Code:��ɫֵ
void LCD_WriteRAM(u16 RGB_Code)
{							    
	*(volatile u16 *) (LCD_DATA) = RGB_Code;//дʮ��λGRAM
}
//��ILI93xx����������ΪGBR��ʽ��������д���ʱ��ΪRGB��ʽ��
//ͨ���ú���ת��
//c:GBR��ʽ����ɫֵ
//����ֵ��RGB��ʽ����ɫֵ
u16 LCD_BGR2RGB(u16 c)
{
	u16  r,g,b,rgb;   
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f;
	r=(c>>11)&0x1f;	 
	rgb=(b<<11)+(g<<5)+(r<<0);		 
	return(rgb);
} 
//��mdk -O1ʱ���Ż�ʱ��Ҫ����
//��ʱi
void opt_delay(u8 i)
{
	while(i--);
}

//���ù��λ��
//Xpos:������
//Ypos:������
static void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
	
		LCD_WR_REG(lcddev.setxcmd);
		LCD_WR_DATA(Xpos>>8); 
		LCD_WR_DATA(Xpos&0XFF);	 
	  LCD_WR_DATA(1);
	  LCD_WR_DATA(223);
		
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(Ypos>>8); 
		LCD_WR_DATA(Ypos&0XFF);
	  LCD_WR_DATA(1);
	  LCD_WR_DATA(63);
	
}

void set_windows(u16 Xpos, u16 Ypos)
{
		LCD_WR_REG(lcddev.setxcmd);
    Xpos=lcddev.width-1-Xpos;
		LCD_WR_DATA(Xpos>>8); 
		LCD_WR_DATA(Xpos&0XFF);	 
	  LCD_WR_DATA(480>>8);
	  LCD_WR_DATA((480-1)&0xff);
		
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(Ypos>>8); 
		LCD_WR_DATA(Ypos&0XFF);
	  LCD_WR_DATA((320-1)>>8);
	  LCD_WR_DATA((320-1)&0xff);
}


//��������
//color:Ҫ���������ɫ
 void LCD_Clear(u16 color)
{
	u32 index=0;      
	u32 totalpoint=lcddev.width;
	totalpoint*=lcddev.height; 	//�õ��ܵ���
	LCD_SetCursor(0x0000,0x0000);	//���ù��λ�� 
	LCD_WriteRAM_Prepare();     //��ʼд��GRAM	 	  
	for(index=0;index<totalpoint;index++)
	{
		*(volatile u16 *) (LCD_DATA)=color;	   
	}
}


//����LCD���Զ�ɨ�跽��
//ע��:�����������ܻ��ܵ��˺������õ�Ӱ��(������9341/6804����������),
//����,һ������ΪL2R_U2D����,�������Ϊ����ɨ�跽ʽ,���ܵ�����ʾ������.
//dir:0~7,����8������(���嶨���lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341��IC�Ѿ�ʵ�ʲ���	   	   
void LCD_Scan_Dir(u8 dir)
{
	u16 regval=0;
	u8 dirreg=0;
	u16 temp;  
	if(lcddev.dir==1&&lcddev.id!=0X6804)//����ʱ����6804���ı�ɨ�跽��
	{			   
		switch(dir)//����ת��
		{
			case 0:dir=6;break;
			case 1:dir=7;break;
			case 2:dir=4;break;
			case 3:dir=5;break;
			case 4:dir=1;break;
			case 5:dir=0;break;
			case 6:dir=3;break;
			case 7:dir=2;break;	     
		}
	}
	if(lcddev.id==0x9341||lcddev.id==0X6804||lcddev.id==0X9488)//9341/6804,������
	{
		switch(dir)
		{
			case L2R_U2D://������,���ϵ���
				regval|=(0<<7)|(0<<6)|(0<<5); 
				break;
			case L2R_D2U://������,���µ���
				regval|=(1<<7)|(0<<6)|(0<<5); 
				break;
			case R2L_U2D://���ҵ���,���ϵ���
				regval|=(0<<7)|(1<<6)|(0<<5); 
				break;
			case R2L_D2U://���ҵ���,���µ���
				regval|=(1<<7)|(1<<6)|(0<<5); 
				break;	 
			case U2D_L2R://���ϵ���,������
				regval|=(0<<7)|(0<<6)|(1<<5); 
				break;
			case U2D_R2L://���ϵ���,���ҵ���
				regval|=(0<<7)|(1<<6)|(1<<5); 
				break;
			case D2U_L2R://���µ���,������
				regval|=(1<<7)|(0<<6)|(1<<5); 
				break;
			case D2U_R2L://���µ���,���ҵ���
				regval|=(1<<7)|(1<<6)|(1<<5); 
				break;	 
		}
		dirreg=0X36;
 		regval|=0X08;//BGR   
		if(lcddev.id==0X6804)regval|=0x02;//6804��BIT6��9341�ķ���	   
		LCD_WriteReg(dirreg,regval);
 		if(regval&0X20)
		{
			if(lcddev.width<lcddev.height)//����X,Y
			{
				temp=lcddev.width;
				lcddev.width=lcddev.height;
				lcddev.height=temp;
 			}
		}else  
		{
			if(lcddev.width>lcddev.height)//����X,Y
			{
				temp=lcddev.width;
				lcddev.width=lcddev.height;
				lcddev.height=temp;
 			}
		}  
		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(0);LCD_WR_DATA(0);
		LCD_WR_DATA((lcddev.width-1)>>8);LCD_WR_DATA((lcddev.width-1)&0XFF);
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(0);LCD_WR_DATA(0);
		LCD_WR_DATA((lcddev.height-1)>>8);LCD_WR_DATA((lcddev.height-1)&0XFF);  
  	}else 
	{
		switch(dir)
		{
			case L2R_U2D://������,���ϵ���
				regval|=(1<<5)|(1<<4)|(0<<3); 
				break;
			case L2R_D2U://������,���µ���
				regval|=(0<<5)|(1<<4)|(0<<3); 
				break;
			case R2L_U2D://���ҵ���,���ϵ���
				regval|=(1<<5)|(0<<4)|(0<<3);
				break;
			case R2L_D2U://���ҵ���,���µ���
				regval|=(0<<5)|(0<<4)|(0<<3); 
				break;	 
			case U2D_L2R://���ϵ���,������
				regval|=(1<<5)|(1<<4)|(1<<3); 
				break;
			case U2D_R2L://���ϵ���,���ҵ���
				regval|=(1<<5)|(0<<4)|(1<<3); 
				break;
			case D2U_L2R://���µ���,������
				regval|=(0<<5)|(1<<4)|(1<<3); 
				break;
			case D2U_R2L://���µ���,���ҵ���
				regval|=(0<<5)|(0<<4)|(1<<3); 
				break;	 
		}
		if(lcddev.id==0x8989)//8989 IC
		{
			dirreg=0X11;
			regval|=0X6040;	//65K   
	 	}else//��������IC		  
		{
			dirreg=0X03;
			regval|=1<<12;  
		}
		LCD_WriteReg(dirreg,regval);
	}
}   



 void FSMC_Configuration(void)
{	
	  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  Timing_read,Timing_write;

    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &Timing_read;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &Timing_write;

    FSMC_NORSRAMStructInit(&FSMC_NORSRAMInitStructure);

    /*--------------------- read timings configuration ---------------------*/
    Timing_read.FSMC_AddressSetupTime = 0;  /* [3:0] F2/F4 1~15 HCLK */
    Timing_read.FSMC_AddressHoldTime = 0;   /* [7:4] keep 0x00 in SRAM mode */
    Timing_read.FSMC_DataSetupTime = 5;     /* [15:8] F2/F4 0~255 HCLK  9��Ч*/
    /* [19:16] Time between NEx high to NEx low (BUSTURN HCLK) */
    Timing_read.FSMC_BusTurnAroundDuration = 0;
    Timing_read.FSMC_CLKDivision = 0; /* [24:20] keep 0x00 in SRAM mode  */
    Timing_read.FSMC_DataLatency = 0; /* [27:25] keep 0x00 in SRAM mode  */
    Timing_read.FSMC_AccessMode = FSMC_AccessMode_B;

    /*--------------------- write timings configuration ---------------------*/
    Timing_write.FSMC_AddressSetupTime = 0;  /* [3:0] F2/F4 1~15 HCLK */
    Timing_write.FSMC_AddressHoldTime = 0;   /* [7:4] keep 0x00 in SRAM mode */
    Timing_write.FSMC_DataSetupTime = 0;     /* [15:8] F2/F4 0~255 HCLK 9��Ч*/
    /* [19:16] Time between NEx high to NEx low (BUSTURN HCLK) */
    Timing_write.FSMC_BusTurnAroundDuration = 0;
    Timing_write.FSMC_CLKDivision = 0; /* [24:20] keep 0x00 in SRAM mode  */
    Timing_write.FSMC_DataLatency = 0; /* [27:25] keep 0x00 in SRAM mode  */
    Timing_write.FSMC_AccessMode = FSMC_AccessMode_B;


    /* Color LCD configuration ------------------------------------
       LCD configured as follow:
          - Data/Address MUX = Disable
          - Memory Type = SRAM
          - Data Width = 16bit
          - Write Operation = Enable
          - Extended Mode = Enable
          - Asynchronous Wait = Disable */
    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);
		
//	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
//  FSMC_NORSRAMTimingInitTypeDef  FSMC_NORSRAMTimingInitStructure;	

//	
//	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

//	FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 0;
//	FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0x0;
//	FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 0x08;
//	FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0x0;
//	FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 0x0;
//	FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0x00;
//	FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_B;
//	
//	
//	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;
//	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
//	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
//	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable; 
//	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
//	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
//	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
//	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
//	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
//  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;

//	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 
//	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);
}
/*--------------����IO����----------------------*/
 void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOG, ENABLE);	

//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOG, GPIO_PinSource10, GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC); 
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

//	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FSMC);
//	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FSMC);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4  | GPIO_Pin_5   | 
//	GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15;															
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;	
//	GPIO_Init(GPIOD, &GPIO_InitStructure);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7  | GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 |
//  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
//	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	
	GPIO_Init(GPIOH, &GPIO_InitStructure);		
	

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	
//	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
}
#define RST_HIGH		GPIOH->BSRRL = GPIO_Pin_9
#define RST_LOW			GPIOH->BSRRH = GPIO_Pin_9
 void LCD_Reset(void)
{
	RST_HIGH;
	Delay_Nclk(4000);
	RST_LOW;
	Delay_Nclk(4000);
	RST_HIGH;
	Delay_Nclk(4000);	
}
void Enter_Standby(void)
{
	LCD_WR_REG( 0x26); LCD_WR_DATA (0x38); //
	LCD_WR_REG( 0x26); LCD_WR_DATA (0x28); //
	LCD_WR_REG( 0x26); LCD_WR_DATA (0x00); //

	LCD_WR_REG( 0x43); LCD_WR_DATA (0x00); //
	LCD_WR_REG( 0x1B); LCD_WR_DATA (0x00); // 
	LCD_WR_REG( 0x1B); LCD_WR_DATA (0x08); // 
	LCD_WR_REG( 0x1C); LCD_WR_DATA (0x00); // 
	LCD_WR_REG( 0x90); LCD_WR_DATA (0x00); //
	LCD_WR_REG( 0x1B); LCD_WR_DATA (0x09); // 
	LCD_WR_REG( 0x19); LCD_WR_DATA (0x48); // 
}
void Exit_Standby(void)
{
	LCD_WR_REG( 0x19); LCD_WR_DATA (0x49); // 
	LCD_WR_REG( 0x1B); LCD_WR_DATA (0x08); // 
	LCD_WR_REG( 0x20); LCD_WR_DATA (0x40); // BT=0100
	LCD_WR_REG( 0x1D); LCD_WR_DATA (0x07); // VC1=111
	LCD_WR_REG( 0x1E); LCD_WR_DATA (0x00); // VC3=000
	LCD_WR_REG( 0x1F); LCD_WR_DATA (0x03); // VRH=0011
	LCD_WR_REG( 0x44); LCD_WR_DATA (0x50); // VCM=101 0000
	LCD_WR_REG( 0x45); LCD_WR_DATA (0x13); // VDV=1 0001
	LCD_WR_REG( 0x1C); LCD_WR_DATA (0x04); // AP=100
	LCD_WR_REG( 0x1B); LCD_WR_DATA (0x18); // 
	LCD_WR_REG( 0x1B); LCD_WR_DATA (0x10); // 
	LCD_WR_REG( 0x43); LCD_WR_DATA (0x80); //set VCOMG=1
	LCD_WR_REG( 0x90); LCD_WR_DATA (0x7f); //
	LCD_WR_REG( 0x26); LCD_WR_DATA (0x04); //GON=0, DTE=0, D=01
	LCD_WR_REG( 0x26); LCD_WR_DATA (0x24); //GON=1, DTE=0, D=01
	LCD_WR_REG( 0x26); LCD_WR_DATA (0x2C); //GON=1, DTE=0, D=11
	LCD_WR_REG( 0x26); LCD_WR_DATA (0x3C); //GON=1, DTE=1, D=11
}	


void lcd_Initializtion(void)
{
	GPIO_Configuration();
  FSMC_Configuration();
	LCD_Reset();
	LCD_WriteReg(0x0000,0x0001);
	delay_ms(50); // delay 50 ms 
  lcddev.id = LCD_ReadReg(0x0000); 
	if(lcddev.id<0XFF||lcddev.id==0XFFFF||lcddev.id==0X9300)//����ID����ȷ,����lcddev.id==0X9300�жϣ���Ϊ9341��δ����λ������»ᱻ����9300
	{	
 		//����9341 ID�Ķ�ȡ		
		LCD_WR_REG(0XD3);				   
		LCD_RD_DATA(); 				//dummy read 	
 		LCD_RD_DATA();   	    	//����0X00
  	lcddev.id=LCD_RD_DATA();   	//��ȡ93								   
 		lcddev.id<<=8;
		lcddev.id|=LCD_RD_DATA();  	//��ȡ41 	   			   
 		if(lcddev.id!=0X9341)		//��9341,�����ǲ���6804
		{	
 		LCD_WR_REG(0XBF);				   
		LCD_RD_DATA(); 			//dummy read 	 
	 	LCD_RD_DATA();   	    //����0X01			   
	 	LCD_RD_DATA(); 			//����0XD0 			  	
	  lcddev.id=LCD_RD_DATA();//�������0X68 
		lcddev.id<<=8;
	  lcddev.id|=LCD_RD_DATA();//�������0X04	   	  
 		}
if(lcddev.id!=0X6804)	
{
//����9488 ID�Ķ�ȡ		
		LCD_WR_REG(0XD3);				   
		LCD_RD_DATA();			//dummy read 	
	  
 		LCD_RD_DATA();
  	lcddev.id=LCD_RD_DATA();   	//��ȡ94								   
 		lcddev.id<<=8;
		lcddev.id|=LCD_RD_DATA();  	//��ȡ88 	   			   
		}			
	
if(lcddev.id!=0X9341&&lcddev.id!=0X6804&&lcddev.id!=0X9488)lcddev.id=0x9341;//����������ʶ��9341 	     
	}
	
 
	
	 lcddev.id =0x9488;
	  rt_kprintf(" LCD ID:%x\r\n",lcddev.id); //��ӡLCD ID  
  if(lcddev.id == 0x7783)
		{
		 LCD_WR_REG(0XD3);
		 LCD_WriteReg(0x0001,0x0100); 
		 LCD_WriteReg(0x0002,0x0700); 
		 LCD_WriteReg(0x0003,0x1030); 
		 LCD_WriteReg(0x0004,0x0000); 
		 LCD_WriteReg(0x0008,0x0302);  
		 LCD_WriteReg(0x0009,0x0000);
		 LCD_WriteReg(0x000A,0x0000); 
		 LCD_WriteReg(0x000C,0x0000); 
		 LCD_WriteReg(0x000D,0x0000);
		 LCD_WriteReg(0x000F,0x0000);/**/
		 LCD_WriteReg(0x0010,0x10e0);   
		 LCD_WriteReg(0x0011,0x0007);  
		 LCD_WriteReg(0x0012,0x0000);  
		 LCD_WriteReg(0x0013,0x0000); 
		 LCD_WriteReg(0x0010,0x1290);   
		 LCD_WriteReg(0x0011,0x0227);
		 LCD_WriteReg(0x0012,0x001d); //0x001b
		 LCD_WriteReg(0x0013,0x1500);
		 LCD_WriteReg(0x0029,0x0018); 
		 LCD_WriteReg(0x002B,0x000D); 
		 LCD_WriteReg(0x0030,0x0004);
		 LCD_WriteReg(0x0031,0x0507);
		 LCD_WriteReg(0x0032,0x0303);// 0006
		 LCD_WriteReg(0x0035,0x0204);
		 LCD_WriteReg(0x0036,0x0707);
		 LCD_WriteReg(0x0037,0x0000); 
		 LCD_WriteReg(0x0038,0x0507);//0200
		 LCD_WriteReg(0x0039,0x0303); 
		 LCD_WriteReg(0x003C,0x0004);// 0504
		 LCD_WriteReg(0x003D,0x0707); 
		 LCD_WriteReg(0x0060,0x0700); 
		 LCD_WriteReg(0x0061,0x0001); 
		 LCD_WriteReg(0x006A,0x0000); 
		 LCD_WriteReg(0x0090,0x003a); 
		 LCD_WriteReg(0x0095,0x021e); 
		 LCD_WriteReg(0x00ff,0x0001); 
		 LCD_WriteReg(0x00b0,0x2b0c); 
		 LCD_WriteReg(0x00b1,0x0000);
		 LCD_WriteReg(0x00ff,0x0000); 
		 LCD_WriteReg(0x0007,0x0133);
		 lcddev.wramcmd=0x22;
		#if USE_HORIZONTAL==1	//ʹ�ú���	  
			lcddev.dir=1;//����
			lcddev.width=320;
			lcddev.height=240;
			lcddev.setxcmd=0x21;
			lcddev.setycmd=0x20;			
			LCD_WriteReg(0x03,0x1038);
			LCD_WriteReg(0x0060,0x2700);
		#else//����
			lcddev.dir=0;//����				 	 		
			lcddev.width=240;
			lcddev.height=320;
			lcddev.setxcmd=0x20;
			lcddev.setycmd=0x21;	
			LCD_WriteReg(0x03,0x1030);
			LCD_WriteReg(0x0060,0xA700);
		#endif
	}	
		else if(lcddev.id == 0x9488)
		{
	LCD_WR_REG(0xE0); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x0D); 
	LCD_WR_DATA(0x1B); 
	LCD_WR_DATA(0x0A); 
	LCD_WR_DATA(0x3c); 
	LCD_WR_DATA(0x78); 
	LCD_WR_DATA(0x4A); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x0E); 
	LCD_WR_DATA(0x09); 
	LCD_WR_DATA(0x1B); 
	LCD_WR_DATA(0x1e); 
	LCD_WR_DATA(0x0f);  
	
	LCD_WR_REG(0xE1); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x22); 
	LCD_WR_DATA(0x24); 
	LCD_WR_DATA(0x06); 
	LCD_WR_DATA(0x12); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x36); 
	LCD_WR_DATA(0x47); 
	LCD_WR_DATA(0x47); 
	LCD_WR_DATA(0x06); 
	LCD_WR_DATA(0x0a); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x30); 
	LCD_WR_DATA(0x37); 
	LCD_WR_DATA(0x0f); 
	
	LCD_WR_REG(0xC0); 
	LCD_WR_DATA(0x10); 
	LCD_WR_DATA(0x10); 
	
	LCD_WR_REG(0xC1); 
	LCD_WR_DATA(0x41); 
	
	LCD_WR_REG(0xC5); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x22); 
	LCD_WR_DATA(0x80); 
	
	LCD_WR_REG(0x36); 
	LCD_WR_DATA(0xC8); //ɨ�跽��Ĵ���  MY MX MV ML BGR MH X X
	
	LCD_WR_REG(0x3A); //Interface Mode Control
	LCD_WR_DATA(0x55);
		
	LCD_WR_REG(0XB0);  //Interface Mode Control  
	LCD_WR_DATA(0x00); 
	
	LCD_WR_REG(0xB1);   //Frame rate 70HZ  
	LCD_WR_DATA(0xB0); 
	LCD_WR_DATA(0x11); 

	LCD_WR_REG(0xB4); 
	LCD_WR_DATA(0x02);   
	LCD_WR_REG(0xB6); //RGB/MCU Interface Control
	LCD_WR_DATA(0x06); 
	LCD_WR_DATA(0x02); 
	
	LCD_WR_REG(0xB7); 
	LCD_WR_DATA(0xC6); 
	
	LCD_WR_REG(0XBE);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x04);
	
	LCD_WR_REG(0xE9); 
	LCD_WR_DATA(0x00);
	
	LCD_WR_REG(0XF7);    
	LCD_WR_DATA(0xA9); 
	LCD_WR_DATA(0x51); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x82);
	

	LCD_WR_REG(0x11); 
	delay_ms(120); 
	LCD_WR_REG(0x29);

	
  lcddev.wramcmd=0x2c;
	
		#if USE_HORIZONTAL==1	//ʹ�ú���	  
			lcddev.dir=1;//����
			lcddev.width=480;
			lcddev.height=320;
			lcddev.setxcmd=0x2B;
			lcddev.setycmd=0x2A;			
			LCD_WriteReg(R7, 0x0173);
			LCD_SetCursor(0x00,0x0000);
		#else//����
			lcddev.dir=0;//����				 	 		
			lcddev.width=320;
			lcddev.height=480;
			lcddev.setxcmd=0x2A;
			lcddev.setycmd=0x2B;	
			LCD_WriteReg(R7, 0x0173);
      LCD_SetCursor(0x00,0x0000);
		#endif

		}
			LCD_LED=1;					//��������
			LCD_Clear(Cyan);	
			delay_ms(10000);
			
}


//����
//x,y:����
//POINT_COLOR:�˵����ɫ
//u16 POINT_COLOR;
void LCD_DrawPoint(u16 x,u16 y,u16 POINT_COLOR)
{
			LCD_SetCursor(x,y);//���ù��λ�� 
			LCD_WriteRAM_Prepare();	//��ʼд��GRAM
			LCD_WR_DATA(POINT_COLOR); 
}

//����
//x1,y1:�������
//x2,y2:�յ�����  
 void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2,u16 POINT_COLOR)
{
			u16 t; 
			int xerr=0,yerr=0,delta_x,delta_y,distance; 
			int incx,incy,uRow,uCol; 
			delta_x=x2-x1; //������������ 
			delta_y=y2-y1; 
			uRow=x1; 
			uCol=y1; 
			if(delta_x>0)incx=1; //���õ������� 
			else if(delta_x==0)incx=0;//��ֱ�� 
			else {incx=-1;delta_x=-delta_x;} 
			if(delta_y>0)incy=1; 
			else if(delta_y==0)incy=0;//ˮƽ�� 
			else{incy=-1;delta_y=-delta_y;} 
			if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
			else distance=delta_y; 
			for(t=0;t<=distance+1;t++ )//������� 
			{  
				LCD_DrawPoint(uRow,uCol,POINT_COLOR);//���� 
				xerr+=delta_x ; 
				yerr+=delta_y ; 
				if(xerr>distance) 
				{ 
					xerr-=distance; 
					uRow+=incx; 
				} 
				if(yerr>distance) 
				{ 
					yerr-=distance; 
					uCol+=incy; 
				} 
			}  
}

/****************************************************************************
* LCD��س���:Һ���ĵײ�������STenwin֮��Ľӿ�
****************************************************************************/

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetHeight
*	����˵��: ��ȡLCD�ֱ���֮�߶�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint16_t LCD_GetHeight(void)
{
	return LCD_HEIGHT;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetWidth
*	����˵��: ��ȡLCD�ֱ���֮���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint16_t LCD_GetWidth(void)
{
	return LCD_WIDTH;
}


/* ��ȡ���ص���ɫ */
uint16_t get_pixel(int x, int y)
{
		uint16_t R = 0, G = 0, B = 0 ;
	
		unsigned short p;
		u16 r=0,g=0,b=0;
		if(x>=lcddev.width||y>=lcddev.height){p = 0;return 0;};	   
	
	  LCD_SetCursor(x,y); 
		
		LCD_WR_REG(0X2E);
					
    R=*(volatile u16 *) (LCD_DATA); 
		R=*(volatile u16 *) (LCD_DATA);  
    B=*(volatile u16 *) (LCD_DATA);  
    G=*(volatile u16 *) (LCD_DATA);  		
		
    return (((R >> 11) << 11) | ((G >> 10 ) << 5) | (B >> 11));

}




/****************************************************************************
* LCD��س���:Һ���ĵײ�������rtGui֮��Ľӿ�
****************************************************************************/


/*  �������ص� ��ɫ,X,Y */
void rt_hw_lcd_set_pixel(const char* pixel, int x, int y)
{			
	   LCD_SetCursor(x,y);		//���ù��λ�� 	
	 *(volatile u16 *) (LCD_CMD)=lcddev.wramcmd;	 
	 *(volatile u16 *) (LCD_DATA) = *(rt_uint16_t*)pixel;
}


/* ��ȡ���ص���ɫ */
void rt_hw_lcd_get_pixel(char* pixel, int x, int y)
{
		unsigned short p;
		u16 r=0,g=0,b=0;
		if(x>=lcddev.width||y>=lcddev.height){p = 0;return;};	   
	
	  LCD_SetCursor(x,y); 
		
		LCD_WR_REG(0X2E);
	
		if(*(volatile u16 *) (LCD_DATA))r=0; 						
 
		r=*(volatile u16 *) (LCD_DATA);  		  						
		
		*(rt_uint16_t*)pixel=r;
	
//			unsigned short p;
//		u16 r=0,g=0,b=0;
//		if(x>=lcddev.width||y>=lcddev.height){p = 0;return;};	   
//	
//	  LCD_SetCursor(x,y); 
//		
//		LCD_WR_REG(0X2E);
//	
//		if(*(volatile u16 *) (LCD_DATA))r=0; 		   
//		opt_delay(2);	  
//		r=*(volatile u16 *) (LCD_DATA);  		  	

//	  opt_delay(2);	  
//		b=*(volatile u16 *) (LCD_DATA); 
//		g=r&0XFF;//
//		g<<=8;

//		p = (((r>>11)<<11)|((g>>10)<<5)|(b>>11));	
//		*(rt_uint16_t*)pixel = p;

}

/* ��ˮƽ�� */
void rt_hw_lcd_draw_hline(const char* pixel, int x1, int x2, int y)
{
       *(volatile u16 *) (LCD_CMD)=(lcddev.setxcmd); 

		 *(volatile u16 *) (LCD_DATA)=(x1>>8); 
		 *(volatile u16 *) (LCD_DATA)=(x1&0XFF);	 
	   *(volatile u16 *) (LCD_DATA)=((x2)>>8);
	   *(volatile u16 *) (LCD_DATA)=((x2)&0xff);

		*(volatile u16 *) (LCD_CMD)=(lcddev.setycmd); 
	 
		 *(volatile u16 *) (LCD_DATA)=(y>>8); 
		 *(volatile u16 *) (LCD_DATA)=(y&0XFF);
	   *(volatile u16 *) (LCD_DATA)=((y)>>8);
	   *(volatile u16 *) (LCD_DATA)=((y)&0xff);
		
    *(volatile u16 *) (LCD_CMD)=lcddev.wramcmd;	 
    while (x1 < x2)
    {
     *(volatile u16 *) (LCD_DATA) = (*(rt_uint16_t*)pixel);
     x1++;
    }
}

/* ��ֱ�� */
void rt_hw_lcd_draw_vline(const char* pixel, int x, int y1, int y2)
{

    *(volatile u16 *) (LCD_CMD)=(lcddev.setxcmd); 
	
		 *(volatile u16 *) (LCD_DATA)=(x>>8); 
		 *(volatile u16 *) (LCD_DATA)=(x&0XFF);	 
	   *(volatile u16 *) (LCD_DATA)=((x)>>8);
	   *(volatile u16 *) (LCD_DATA)=((x)&0xff);

		*(volatile u16 *) (LCD_CMD)=(lcddev.setycmd); 
	 
		 *(volatile u16 *) (LCD_DATA)=(y1>>8); 
		 *(volatile u16 *) (LCD_DATA)=(y1&0XFF);
	   *(volatile u16 *) (LCD_DATA)=((y2)>>8);
	   *(volatile u16 *) (LCD_DATA)=((y2)&0xff);

	  *(volatile u16 *) (LCD_CMD)=lcddev.wramcmd;	 
	
			    while (y1 < y2)
    {
        *(volatile u16 *) (LCD_DATA) = (*(rt_uint16_t*)pixel);
        y1++;
    }


}
static u8 i,j;
/*ˮƽ��*/
void rt_hw_lcd_draw_blit_line(const char* pixels, int x, int y, rt_size_t size)
{
	  rt_uint16_t *ptr;

	  ptr = (rt_uint16_t*)pixels;
    *(volatile u16 *) (LCD_CMD)=(lcddev.setxcmd); 
		*(volatile u16 *) (LCD_DATA)=(x>>8); 
		*(volatile u16 *) (LCD_DATA)=(x&0XFF);	 
	  *(volatile u16 *) (LCD_DATA)=((x+size)>>8);
	  *(volatile u16 *) (LCD_DATA)=((x+size)&0xff);

		*(volatile u16 *) (LCD_CMD)=(lcddev.setycmd); 
	  
		 *(volatile u16 *) (LCD_DATA)=(y>>8); 
		 *(volatile u16 *) (LCD_DATA)=(y&0XFF);
	   *(volatile u16 *) (LCD_DATA)=((y)>>8);
	   *(volatile u16 *) (LCD_DATA)=((y)&0xff);

	 *(volatile u16 *) (LCD_CMD)=lcddev.wramcmd;	 


	{
		 while (size)
    {
    *(volatile u16 *) (LCD_DATA) = *(ptr++);
		size--;

    }
	}
	
}


struct rt_device_graphic_ops lcd_ili_ops =
{
	  rt_hw_lcd_set_pixel,
	  rt_hw_lcd_get_pixel,
	  rt_hw_lcd_draw_hline,
	  rt_hw_lcd_draw_vline,
	  rt_hw_lcd_draw_blit_line
};

static rt_err_t lcd_init(rt_device_t dev)
{
	  return RT_EOK;
}

static rt_err_t lcd_open(rt_device_t dev, rt_uint16_t oflag)
{
	  return RT_EOK;
}

static rt_err_t lcd_close(rt_device_t dev)
{
	  return RT_EOK;
}

static rt_err_t lcd_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	switch (cmd)
	{
	case RTGRAPHIC_CTRL_GET_INFO:
		{
			struct rt_device_graphic_info *info;

			info = (struct rt_device_graphic_info*) args;
			RT_ASSERT(info != RT_NULL);

			info->bits_per_pixel = BPP;
			info->pixel_format =RTGRAPHIC_PIXEL_FORMAT_RGB565;// RTGRAPHIC_PIXEL_FORMAT_BGR565;RTGRAPHIC_PIXEL_FORMAT_RGB565P
			info->framebuffer = RT_NULL;
			info->width = LCD_WIDTH;
			info->height = LCD_HEIGHT;
		}
		break;

	case RTGRAPHIC_CTRL_RECT_UPDATE:
		/* nothong to be done */
		break;

	default:
		break;
	}

	return RT_EOK;
}

void rt_hw_lcd_init(void)
{

	/* register lcd device */
	_lcd_device.type  = RT_Device_Class_Graphic;
	_lcd_device.init  = lcd_init;
	_lcd_device.open  = lcd_open;
	_lcd_device.close = lcd_close;
	_lcd_device.control = lcd_control;
	_lcd_device.read  = RT_NULL;
	_lcd_device.write = RT_NULL;

	/* set user privated data */

	_lcd_device.user_data = &lcd_ili_ops;
  lcd_Initializtion();

    /* register graphic device driver */
	rt_device_register(&_lcd_device, "lcd",
	RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
	LCD_LED = 1;

}




