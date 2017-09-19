/*
*********************************************************************************************************
*                                                MinOS
*                                          The Real-Time Kernel
*                                             CORE FUNCTIONS
*
*                              (c) Copyright 2015-2020, ZH, Windy Albert
*                                           All Rights Reserved
*
* File    : MINOS.h
* By      : Windy Albert & Jean J. Labrosse
* Version : V1.00 [From.V2.86]
*
*********************************************************************************************************
*/

#ifndef   _MINOS_H
#define   _MINOS_H

#include <minos_cfg.h>
#include "stm32f10x.h"
/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

//typedef unsigned char  BOOLEAN;
//typedef unsigned char  uint8_t;                    /* Unsigned  8 bit quantity                           */
//typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
//typedef unsigned short uint16_t;                   /* Unsigned 16 bit quantity                           */
//typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
//typedef unsigned int   uint32_t;                   /* Unsigned 32 bit quantity                           */
//typedef signed   int   INT32S;                   /* Signed   32 bit quantity                           */

//typedef unsigned int   uint32_t;                   /* Each stack entry is 32-bit wide                    */
//typedef unsigned int   uint32_t;                /* Define size of CPU status register (PSR = 32 bits) */

/*
*********************************************************************************************************
*                                              Cortex-M1
*                                      Critical Section Management
*
* Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*             would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*             disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
*             disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*             into the CPU's status register.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             MISCELLANEOUS
*********************************************************************************************************
*/



/*$PAGE*/
/*
*********************************************************************************************************
*                                          TASK CONTROL BLOCK
*********************************************************************************************************
*/



/*$PAGE*/
/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

//OS_EXT  uint8_t  const  		OSUnMapTbl[256];

//OS_EXT  uint8_t             OSIntNesting;             /* Interrupt nesting level                         */

//OS_EXT  uint8_t             OSPrioCur;                /* Priority of current task                        */
//OS_EXT  uint8_t             OSPrioHighRdy;            /* Priority of highest priority task               */

//OS_EXT  uint8_t             OSRdyGrp;                        /* Ready list group                         */
//OS_EXT  uint8_t             OSRdyTbl[OS_RDY_TBL_SIZE];       /* Table of tasks which are ready to run    */

//OS_EXT  uint32_t            OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE];      /* Idle task stack                */

//OS_EXT  OS_TCB           *OSTCBCur;                        /* Pointer to currently running TCB         */
//OS_EXT  OS_TCB           *OSTCBFreeList;                   /* Pointer to list of free TCBs             */
//OS_EXT  OS_TCB           *OSTCBHighRdy;                    /* Pointer to highest priority TCB R-to-R   */
//OS_EXT  OS_TCB           *OSTCBList;                       /* Pointer to doubly linked list of TCBs    */
//OS_EXT  OS_TCB           *OSTCBPrioTbl[OS_LOWEST_PRIO + 1];/* Table of pointers to created TCBs        */
//OS_EXT  OS_TCB            OSTCBTbl[OS_MAX_TASKS + OS_N_SYS_TASKS];   /* Table of TCBs                  */


/*$PAGE*/
/*
*********************************************************************************************************
*                                            TASK MANAGEMENT
*********************************************************************************************************
*/

uint8_t         OSTaskCreate            (void           (*task)(void),
                                       uint32_t          *ptos,
                                       uint8_t            prio);

void          OSTimeDly               (uint16_t           ticks);
void          OSTimeTick              (void);

void          OSInit                  (void);
void          OSIntEnter              (void);
void          OSIntExit               (void);
void          OSStart                 (void);





#endif


/********************* (C) COPYRIGHT 2015 Windy Albert **************************** END OF FILE ********/

