/**
  ******************************************************************************
  * @file    main.c 
  * @author  Windy Albert
  * @version V1.0
  * @date    26-August-2015
  * @brief   Main program body.
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "minos.h"	                                 /* Header file for MinOS */

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/************************    Task00 Configuration    **************************/
#define  TASK00_PRIO        0                          /* The priority of task */
#define  TASK00_STK_SIZE   80       /* The stack size of task, NO LESS than 40 */
uint32_t Task00_Stk[
         TASK00_STK_SIZE];                                /* The stack of task */
void     Task00(void);


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
	SysTick_Config(SystemCoreClock/1000);
	
	OSInit();

/************* Task00 Creation ************************************************/
	OSTaskCreate(Task00,
                &Task00_Stk[
                 TASK00_STK_SIZE-1],
                 TASK00_PRIO);
    
	OSStart();
    return 0;

}


/******************* (C) COPYRIGHT 2015 Windy Albert************END OF FILE****/
