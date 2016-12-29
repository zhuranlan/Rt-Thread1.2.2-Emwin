#include <rtthread.h>
#include "board.h"
#include <GPIO.h>

#ifdef RT_USING_DFS
	#include <dfs_init.h>
	#include <dfs_elm.h>
	#include <dfs_fs.h>
	extern void rt_hw_spi1_init(void);
	extern void rt_hw_sdcard_init(void);
#endif


#ifdef RT_USING_USB_DEVICE
	extern void rt_hw_usbd_init(void);
	extern rt_err_t rt_usb_device_init(const char* udc_name);
	extern void rt_hw_ramdisk_init(void);
	extern void rt_usb_vcom_init(void);
#endif

void rt_platform_init(void);

#ifdef RT_USING_RTC
#include "stm32f4_rtc.h"
#endif /* RT_USING_RTC */

#ifdef RT_USING_SPI
#include "stm32f20x_40x_spi.h"
#include "spi_flash_w25qxx.h"

/*
SPI1_MOSI: PB5
SPI1_MISO: PA6
SPI1_SCK : PA5

CS0: PI10  SPI FLASH

*/
 void rt_hw_spi1_init(void)  
{  
    /* register spi bus */  
    {  
        static struct stm32_spi_bus stm32_spi;  
        GPIO_InitTypeDef GPIO_InitStructure;  
  
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB, ENABLE);  

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;  
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;  
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  
  
        /*!< SPI SCK pin configuration */  
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;  
        GPIO_Init(GPIOA, &GPIO_InitStructure);  
			  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;  
        GPIO_Init(GPIOB, &GPIO_InitStructure); 
  
        /* Connect alternate function */  
			  GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
  
        stm32_spi_register(SPI1, &stm32_spi, "spi1");  
    }  
  
    /* attach cs */  
    {  
			static struct rt_spi_device spi_device,spi_device_1;  
      static struct stm32_spi_cs  spi_cs,spi_cs_1;  
  
        GPIO_InitTypeDef GPIO_InitStructure;  
  
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;  
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;  
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  
  
        /* spi0: PI10 */  
        spi_cs.GPIOx = GPIOI;  
        spi_cs.GPIO_Pin = GPIO_Pin_10;  
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);  
  
        GPIO_InitStructure.GPIO_Pin = spi_cs.GPIO_Pin;  
        GPIO_SetBits(spi_cs.GPIOx, spi_cs.GPIO_Pin);  
        GPIO_Init(spi_cs.GPIOx, &GPIO_InitStructure);  

        rt_spi_bus_attach_device(&spi_device, "spi1_flash", "spi1", (void*)&spi_cs);   
				
//        /* spi0: PD12 */  
//        spi_cs_1.GPIOx = GPIOD;  
//        spi_cs_1.GPIO_Pin = GPIO_Pin_12;  
//        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);  
//				
//        GPIO_InitStructure.GPIO_Pin = spi_cs_1.GPIO_Pin;  
//        GPIO_SetBits(spi_cs_1.GPIOx, spi_cs_1.GPIO_Pin);  
//        GPIO_Init(spi_cs_1.GPIOx, &GPIO_InitStructure);  
//        rt_spi_bus_attach_device(&spi_device_1, "spi1_net", "spi1", (void*)&spi_cs_1);  
    }  
} 

/*
SPI2_MOSI: PC3
SPI2_MISO: PC2
SPI2_SCK : PI1

CS: PH11  TOUCH
*/
static int rt_hw_spi2_init(void)
{
    /* register spi bus */
    {
        static struct stm32_spi_bus stm32_spi;
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOI, ENABLE);

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

        /*!< SPI SCK pin configuration */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2| GPIO_Pin_3;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
		

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
        GPIO_Init(GPIOI, &GPIO_InitStructure);

        /* Connect alternate function */
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_SPI2);
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2);
        GPIO_PinAFConfig(GPIOI, GPIO_PinSource1, GPIO_AF_SPI2);

        stm32_spi_register(SPI2, &stm32_spi, "spi2");
    }

    /* attach cs */
    {
        static struct rt_spi_device spi_device;
        static struct stm32_spi_cs  spi_cs;

        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

        /* spi_touch: PH11 */
        spi_cs.GPIOx = GPIOH;
        spi_cs.GPIO_Pin = GPIO_Pin_11;
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);

        GPIO_InitStructure.GPIO_Pin = spi_cs.GPIO_Pin;
        GPIO_SetBits(spi_cs.GPIOx, spi_cs.GPIO_Pin);
        GPIO_Init(spi_cs.GPIOx, &GPIO_InitStructure);

        rt_spi_bus_attach_device(&spi_device, "spi2_touch", "spi2", (void*)&spi_cs);
    }


    return 0;
}
INIT_BOARD_EXPORT(rt_hw_spi2_init);
#endif /* RT_USING_SPI */



//#include <rtgui/driver.h>

//#include "setup.h"
//rt_bool_t cali_setup(void)
//{
//    struct setup_items setup;

//    if(setup_load(&setup) == RT_EOK)

//    setup_save(&setup);
//}
#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#include "stm32f4xx_eth.h"
#endif

extern void rt_hw_GPIO_init(void);
extern void cpu_usage_init(void);


static void mco_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* MCO configure */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);
    RCC_MCO1Config(RCC_MCO1Source_HSE, RCC_MCO1Div_1);
}

//#include "lwip/netif.h"
//#include "lwip_comm.h"

extern unsigned char sd_flag;
void rt_platform_init(void)
{
	  rt_hw_GPIO_init();
	  cpu_usage_init();
	
#ifdef RT_USING_RTC
	  {
		 extern void rt_hw_rtc_init(void);
	   rt_hw_rtc_init();
	  }
#endif
		
#ifdef RT_USING_SPI
		{
		rt_hw_spi1_init();
		//rt_hw_spi2_init();
		}
#endif


		
#ifdef RT_USING_DFS
    {  
			extern rt_err_t w25qxx_init(const char * flash_device_name,
                            const char * spi_device_name);
			rt_bool_t mount_flag = RT_FALSE;
        dfs_init();  
		#ifdef RT_USING_DFS_ELMFAT              
        elm_init();
		#endif  

			w25qxx_init("flash0", "spi1_flash"); 
			rt_hw_sdcard_init();
				
			//rt_thread_delay(100);
//		  mkfs("elm", "flash0");
//			mkdir("/SD");
//			mkdir("/RESOURCE");
			
			
//			/*加载字库到flah中*/
//			copy("/SD/resource/HZK16.FNT","/RESOURCE");
//			copy("/SD/resource/HZK12.FNT","/RESOURCE");
//			copy("/SD/resource/HZK24.FNT","/RESOURCE");
//			copy("/SD/resource/HZK48.FNT","/RESOURCE");
//			copy("/SD/resource/UNI2GBK.TBL","/RESOURCE");
//			copy("/SD/resource/GBK2UNI.TBL","/RESOURCE");
    

        if (dfs_mount("flash0", "/", "elm", 0, 0) == 0)
        {
            mount_flag = RT_TRUE;
        }
        else
        {
          rt_kprintf("flash0 mount to / failed!, fatmat and try again!\n");

            if (dfs_mount("flash0", "/", "elm", 0, 0) == 0)
            {
                mount_flag = RT_TRUE;
            }
        }


        if(mount_flag == RT_TRUE)
        {
            rt_kprintf("flash0 mount to / \n");

		#if (RT_DFS_ELM_USE_LFN != 0) && (defined RT_DFS_ELM_CODE_PAGE_FILE)
            {
               extern void ff_convert_init(void);
                ff_convert_init();
            }
		#endif
           
            if (dfs_mount("sd0", "/SD", "elm", 0, 0) == 0)
            {
							  sd_flag=1;
                rt_kprintf("sd0 mount to /SD \n");
							
            }
            else
            {
                rt_kprintf("sd0 mount to /SD failed!\n");
            }
       }
        else
        {
            rt_kprintf("flash0 mount to / failed!\n");
        }
   }
#endif
	 
	 
/*CLK 50MHZ*/
 mco_config();
rt_thread_delay(100);	

			 
 /* LwIP Initialization */
#ifdef RT_USING_LWIP
    {
			#include "httpd.h"
        extern void lwip_sys_init(void);

        
        /* register ethernetif device */
        eth_system_device_init();

        rt_hw_stm32_eth_init();

        /* init lwip system */
        lwip_sys_init();
			
        rt_kprintf("TCP/IP initialized!\n");
			
   	  //tcpserv();//开启TCP服务器
			
//	  #if defined(RT_USING_DFS) && defined(RT_USING_LWIP)
//		  	{
//	  	extern void ftpd_start(void);
//		  /* start ftp server */
//		  rt_kprintf("ftp server begin...\n");
//		  ftpd_start();
//		  rt_kprintf("ftp server started!!\n");
//			}
//		#endif
     httpd_init();
    }
#endif	
			
		  rt_thread_delay(50);
      rt_device_init_all();
		
}
		
	
		
