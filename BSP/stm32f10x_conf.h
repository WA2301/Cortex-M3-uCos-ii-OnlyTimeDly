/**
  ******************************************************************************
  * @file    stm32f10x_conf.h 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Library configuration file.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_CONF_H
#define __STM32F10x_CONF_H

/* Includes ------------------------------------------------------------------*/
/* Uncomment/Comment the line below to enable/disable peripheral header file inclusion */
//#include "stm32f10x_adc.h"
//#include "stm32f10x_bkp.h"
//#include "stm32f10x_can.h"
//#include "stm32f10x_cec.h"
//#include "stm32f10x_crc.h"
//#include "stm32f10x_dac.h"
//#include "stm32f10x_dbgmcu.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_flash.h"
//#include "stm32f10x_fsmc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
//#include "stm32f10x_iwdg.h"
//#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"
//#include "stm32f10x_rtc.h"
//#include "stm32f10x_sdio.h"
//#include "stm32f10x_spi.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
//#include "stm32f10x_wwdg.h"
#include "misc.h" /* High level functions for NVIC and SysTick (add-on to CMSIS functions) */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Public macros -------------------------------------------------------------*/


/* 
	Periph BitBand 
	Example:
	vuc32 *p=GPO_BAND(GPIOF_BASE,6);
	Sbit(PA,1);
 */
#define GPI_BAND(Addr,N)  (vu32*)(0x42200100+((Addr&0xFFFF)<<5)+(N<<2))/**IDR**/
#define GPO_BAND(Addr,N)  (vu32*)(0x42200180+((Addr&0xFFFF)<<5)+(N<<2))/**ODR**/


/**
  ******************************************************************************
	** Read and Write flash
  ******************************************************************************
	**/
	
/** Page:0x00 - 0x7F  Addr: 0x0000-0x01FF[Unit by 32bit] **/

#define p_FLASH(Page,Addr)  ( 0x08000000 + 0x00800*Page + Addr*4 )
void Put_Flash(u8 Page,u8 addr,u32 data);
void Put_Flash_ByAddr(u32 addr,u32 data);

/* 
	RAM BitBand 
	Example:
	vu32 * p=BIT0_BAND(&val);
 */

#define BIT0_BAND(Addr)  	(vu32*)(SRAM_BB_BASE+0x00+((Addr&0xFFFF)<<5))
#define BIT1_BAND(Addr)  	(vu32*)(SRAM_BB_BASE+0x04+((Addr&0xFFFF)<<5))
#define BIT2_BAND(Addr)  	(vu32*)(SRAM_BB_BASE+0x08+((Addr&0xFFFF)<<5))
#define BIT3_BAND(Addr)  	(vu32*)(SRAM_BB_BASE+0x0C+((Addr&0xFFFF)<<5))
#define BIT4_BAND(Addr)  	(vu32*)(SRAM_BB_BASE+0x10+((Addr&0xFFFF)<<5))
#define BIT5_BAND(Addr)  	(vu32*)(SRAM_BB_BASE+0x14+((Addr&0xFFFF)<<5))
#define BIT6_BAND(Addr)  	(vu32*)(SRAM_BB_BASE+0x18+((Addr&0xFFFF)<<5))
#define BIT7_BAND(Addr)  	(vu32*)(SRAM_BB_BASE+0x1C+((Addr&0xFFFF)<<5))



/* Uncomment the line below to expanse the "assert_param" macro in the 
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function which reports 
  *         the name of the source file and the source line number of the call 
  *         that failed. If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */


#endif /* __STM32F10x_CONF_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
