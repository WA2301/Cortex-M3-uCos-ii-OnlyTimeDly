/**
 ******************************************************************************
 * @file    Task00.c
 * @author  Windy Albert
 * @date    08-April-2016
 * @brief   Task to ...
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_app.h"
#include "minos.h"									/* Header file for MinOS */

u32 test;

/**
 * @brief		Task00 program.
 * @function  	None
 * @RunPeriod 	None
 */

void Task00(void) {

	GPIO_InitTypeDef GPIO_InitStructure;

	/** Task Initialize *******************************************************/

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	for (;;) {
		//TODO:

		test++;
		*GPO_BAND(GPIOC_BASE, 13) ^= 1;
		OSTimeDly(800);

		/******************************************************************************/
	} //for
}

/******************* (C) COPYRIGHT 2014 Windy Albert ***********END OF FILE****/
