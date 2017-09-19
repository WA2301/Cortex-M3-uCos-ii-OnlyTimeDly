/*
*********************************************************************************************************
*                                                MinOS
*                                          The Real-Time Kernel
*                                             CORE FUNCTIONS
*
*                              (c) Copyright 2015-2020, ZH, Windy Albert
*                                           All Rights Reserved
*
* File    : MINOS_CORE.C
* By      : Windy Albert & Jean J. Labrosse
* Version : V1.00 [From.V2.86]
*
*********************************************************************************************************
*/

#define  OS_GLOBALS

																			/* OS_EXT is BLANK.  */
#include "stm32f10x.h"/* OS_EXT is BLANK.  */
#include <minos.h>

//#define  OS_CRITICAL_METHOD   3

//#define  uint32_t_GROWTH        1                   /* Stack grows from HIGH to LOW memory on ARM        */

//#if OS_CRITICAL_METHOD == 3                       /* See OS_CPU_A.ASM                                  */
uint32_t  OS_CPU_SR_Save(void);
void       OS_CPU_SR_Restore(uint32_t cpu_sr);

#define  OS_ENTER_CRITICAL()  do{cpu_sr = OS_CPU_SR_Save();}while(0)
#define  OS_EXIT_CRITICAL()   do{OS_CPU_SR_Restore(cpu_sr);}while(0)

//#endif

void       OSCtxSw(void);
void       OSStartHighRdy(void);




//#ifdef   OS_GLOBALS
//#define  OS_EXT
//#else
//#define  OS_EXT  extern
//#endif

//#define  OS_FALSE                     0u
//#define  OS_TRUE                      1u

#define OS_MAX_TASKS  		 OS_LOWEST_PRIO    /* Max. number of tasks in your application, MUST be >= 1 */
#define OS_TASK_IDLE_STK_SIZE    		  80    /* Idle  task stack size (# of uint32_t wide entries)        */

#define  OS_PRIO_SELF              0xFFu                /* Indicate SELF priority                      */
#define  OS_N_SYS_TASKS               1u
#define  OS_TASK_IDLE_PRIO  (OS_LOWEST_PRIO)            /* IDLE      task priority                     */
#define  OS_RDY_TBL_SIZE   ((OS_LOWEST_PRIO) / 8 + 1)   /* Size of ready table                         */

#define  OS_TCB_RESERVED        ((OS_TCB *)1)

/*$PAGE*/
/*
*********************************************************************************************************
*                              TASK STATUS (Bit definition for OSTCBStat)
*********************************************************************************************************
*/
#define  OS_STAT_RDY               0x00u    /* Ready to run                                            */
#define  OS_STAT_PEND_Q            0x04u    /* Pending on queue                                        */
#define  OS_STAT_SUSPEND           0x08u    /* Task is suspended                                       */

#define  OS_STAT_PEND_OK              0u    /* Pending status OK, not pending, or pending complete     */
#define  OS_STAT_PEND_TO              1u    /* Pending timed out                                       */
#define  OS_STAT_PEND_ABORT           2u    /* Pending aborted                                         */


/*
*********************************************************************************************************
*                                             ERROR CODES
*********************************************************************************************************
*/
#define OS_ERR_NONE                   0u
#define OS_ERR_PEND_ISR               2u
#define OS_ERR_TIMEOUT               10u
#define OS_ERR_PEND_ABORT            14u
//#define OS_ERR_Q_FULL                30u
//#define OS_ERR_Q_EMPTY               31u
#define OS_ERR_PRIO_EXIST            40u
#define OS_ERR_TASK_NO_MORE_TCB      66u
#define OS_ERR_TASK_NOT_EXIST        67u

#define  OS_EVENT_TBL_SIZE ((OS_LOWEST_PRIO) / 8 + 1)   /* Size of event table                         */




typedef struct os_tcb {
    uint32_t          *OSTCBStkPtr;           /* Pointer to current top of stack                         */
    struct os_tcb   *OSTCBNext;             /* Pointer to next     TCB in the TCB list                 */
    struct os_tcb   *OSTCBPrev;             /* Pointer to previous TCB in the TCB list                 */


    uint16_t           OSTCBDly;              /* Nbr ticks to delay task or, timeout waiting for event   */
    uint8_t            OSTCBStat;             /* Task      status                                        */

    uint8_t            OSTCBPrio;             /* Task priority (0 == highest)                            */
    uint8_t            OSTCBX;                /* Bit position in group  corresponding to task priority   */
    uint8_t            OSTCBY;                /* Index into ready table corresponding to task priority   */
    uint8_t            OSTCBBitX;             /* Bit mask to access bit position in ready table          */
    uint8_t            OSTCBBitY;             /* Bit mask to access bit position in ready group          */
} OS_TCB;





  uint8_t  const  		OSUnMapTbl[256];

  uint8_t             OSIntNesting;             /* Interrupt nesting level                         */

  uint8_t             OSPrioCur;                /* Priority of current task                        */
  uint8_t             OSPrioHighRdy;            /* Priority of highest priority task               */

  uint8_t             OSRdyGrp;                        /* Ready list group                         */
  uint8_t             OSRdyTbl[OS_RDY_TBL_SIZE];       /* Table of tasks which are ready to run    */

  uint32_t            OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE];      /* Idle task stack                */

  OS_TCB           *OSTCBCur;                        /* Pointer to currently running TCB         */
  OS_TCB           *OSTCBFreeList;                   /* Pointer to list of free TCBs             */
  OS_TCB           *OSTCBHighRdy;                    /* Pointer to highest priority TCB R-to-R   */
  OS_TCB           *OSTCBList;                       /* Pointer to doubly linked list of TCBs    */
  OS_TCB           *OSTCBPrioTbl[OS_LOWEST_PRIO + 1];/* Table of pointers to created TCBs        */
  OS_TCB            OSTCBTbl[OS_MAX_TASKS + OS_N_SYS_TASKS];   /* Table of TCBs                  */




//#define OSCtxSw()  do{SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;}while(0)

/*
*********************************************************************************************************
*                                       PRIORITY RESOLUTION TABLE
*
* Note: Index into table is bit pattern to resolve highest priority
*       Indexed value corresponds to highest priority bit position (i.e. 0..7)
*********************************************************************************************************
*/

uint8_t  const  OSUnMapTbl[256] = {
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x00 to 0x0F     */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x10 to 0x1F     */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x20 to 0x2F     */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x30 to 0x3F     */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x40 to 0x4F     */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x50 to 0x5F     */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x60 to 0x6F     */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x70 to 0x7F     */
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x80 to 0x8F     */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x90 to 0x9F     */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xA0 to 0xAF     */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xB0 to 0xBF     */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xC0 to 0xCF     */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xD0 to 0xDF     */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xE0 to 0xEF     */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0        /* 0xF0 to 0xFF     */
};


uint32_t OS_CPU_SR_Save(void)//±£´æÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷
{
    uint32_t primask;
    
    primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}


void OS_CPU_SR_Restore(uint32_t primask)//»Ö¸´ÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷
{
    __set_PRIMASK(primask);
}


void OSStartHighRdy(void)
{

	SCB->SHP[10] = 0xFF;

    __set_PSP(0);/* Set the PSP to 0 for initial context switch call */

	OSCtxSw();

	__enable_irq(); /*  Enable interrupts at processor level */
    OSCtxSw();
}


void OSCtxSw(void)
{
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}












/*$PAGE*/
/*
*********************************************************************************************************
*                              FIND HIGHEST PRIORITY TASK READY TO RUN
*
* Description: This function is called by other uC/OS-II services to determine the highest priority task
*              that is ready to run.  The global variable 'OSPrioHighRdy' is changed accordingly.
*
* Arguments  : none
*
* Returns    : none
*
*********************************************************************************************************
*/

static  void  OS_SchedNew (void)
{    
    uint8_t   y;
    y             = OSUnMapTbl[OSRdyGrp];
    OSPrioHighRdy = (uint8_t)((y << 3) + OSUnMapTbl[OSRdyTbl[y]]);
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                              SCHEDULER
*
* Description: This function is called by other uC/OS-II services to determine whether a new, high
*              priority task has been made ready to run.  This function is invoked by TASK level code
*              and is not used to reschedule tasks from ISRs (see OSIntExit() for ISR rescheduling).
*
* Arguments  : none
*
* Returns    : none
*
*********************************************************************************************************
*/

static void  OS_Sched (void)
{
    uint32_t  cpu_sr = 0;

    OS_ENTER_CRITICAL();

    if (OSIntNesting == 0)
    {
        OS_SchedNew();
        if (OSPrioHighRdy != OSPrioCur)/* No Ctx Sw if current task is highest rdy     */
        {                   
            OSTCBHighRdy = OSTCBPrioTbl[OSPrioHighRdy];                            
            OSCtxSw();
        }
    }

    OS_EXIT_CRITICAL();
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           ENTER/EXIT ISR
*
* Description: This function is used to notify uC/OS-II that you are about to service an interrupt
*              service routine (ISR).  This allows uC/OS-II to keep track of interrupt nesting and thus
*              only perform rescheduling at the last nested ISR.
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : You MUST invoke OSIntEnter() and OSIntExit() in pair.  In other words, for every call
*              to OSIntEnter() at the beginning of the ISR you MUST have a call to OSIntExit() at the
*              end of the ISR.
*              
*********************************************************************************************************
*/

void  OSIntEnter (void)
{
    if (OSIntNesting < 255u)
    {
        OSIntNesting++;                        /* Increment ISR nesting level */
    }
}

void  OSIntExit (void)
{
    uint32_t  cpu_sr = 0;
    
    OS_ENTER_CRITICAL();

    if (OSIntNesting > 0)                                     /* Prevent OSIntNesting from wrapping       */
    {
        OSIntNesting--;
    }
    
    if (OSIntNesting == 0)                                    /* Reschedule only if all ISRs complete ... */
    {
        OS_SchedNew();
        if (OSPrioHighRdy != OSPrioCur)                       /* No Ctx Sw if current task is highest rdy */
        {
            OSTCBHighRdy  = OSTCBPrioTbl[OSPrioHighRdy];
            OSCtxSw();                      /* Perform interrupt level ctx switch       */
        }
    }

    OS_EXIT_CRITICAL();
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                         PROCESS SYSTEM TICK
*
* Description: This function is used to signal to uC/OS-II the occurrence of a 'system tick' (also known
*              as a 'clock tick').  This function should be called by the ticker ISR but, can also be
*              called by a high priority task.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

void  OSTimeTick (void)
{
    OS_TCB    *ptcb;
        
    uint32_t  cpu_sr = 0;

    ptcb = OSTCBList;                                  /* Point at first TCB in TCB list               */
    while (ptcb->OSTCBPrio != OS_TASK_IDLE_PRIO)     /* Go through all TCBs in TCB list              */
    {
        OS_ENTER_CRITICAL();
        if (ptcb->OSTCBDly != 0)                     /* No, Delayed or waiting for event with TO     */
        {
            if (--ptcb->OSTCBDly == 0)               /* Decrement nbr of ticks to end of delay       */
            {                                                             /* Check for timeout                            */
                if ((ptcb->OSTCBStat & OS_STAT_PEND_Q) != OS_STAT_RDY) 
                {
                    ptcb->OSTCBStat  &= ~(uint8_t)OS_STAT_PEND_Q; 
                    
                }

                

                if ((ptcb->OSTCBStat & OS_STAT_SUSPEND) == OS_STAT_RDY)  /* Is task suspended?       */
                {
                    OSRdyGrp               |= ptcb->OSTCBBitY;             /* No,  Make ready          */
                    OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
                }
            }
        }
        ptcb = ptcb->OSTCBNext;                        /* Point at next TCB in TCB list                */
        OS_EXIT_CRITICAL();
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                DELAY TASK 'n' TICKS   (n from 0 to 65535)
*
* Description: This function is called to delay execution of the currently running task until the
*              specified number of system ticks expires.  This, of course, directly equates to delaying
*              the current task for some time to expire.  No delay will result If the specified delay is
*              0.  If the specified delay is greater than 0 then, a context switch will result.
*
* Arguments  : ticks     is the time delay that the task will be suspended in number of clock 'ticks'.
*                        Note that by specifying 0, the task will not be delayed.
*
* Returns    : none
*********************************************************************************************************
*/
void  OSTimeDly (uint16_t  ticks)
{
    uint8_t      y;
    uint32_t  cpu_sr = 0;

    if (OSIntNesting > 0)                      /* See if trying to call from an ISR                  */
    {
        return;
    }
    if (ticks > 0)                             /* 0 means no delay!                                  */
    {
        OS_ENTER_CRITICAL();
        y            =  OSTCBCur->OSTCBY;        /* Delay current task                                 */
        OSRdyTbl[y] &= ~OSTCBCur->OSTCBBitX;
        if (OSRdyTbl[y] == 0)
        {
            OSRdyGrp &= ~OSTCBCur->OSTCBBitY;
        }
        OSTCBCur->OSTCBDly = ticks;              /* Load ticks in TCB                                  */
        OS_EXIT_CRITICAL();
        OS_Sched();                              /* Find next task to run!                             */
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                              IDLE TASK
*
* Description: This task is internal to uC/OS-II and executes whenever no other higher priority tasks
*              executes because they are ALL waiting for event(s) to occur.
*
* Arguments  : none
*
* Returns    : none
*
*********************************************************************************************************
*/

static void  OS_TaskIdle (void)
{
    for (;;)
    {
        //Do nothing.
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             INITIALIZATION
*
* Description: This function is used to initialize the internals of uC/OS-II and MUST be called prior to
*              creating any uC/OS-II object and, prior to calling OSStart().
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

void  OSInit (void)
{
    uint8_t    i;        
    OS_TCB  *ptcb1,*ptcb2;
        
    OSIntNesting  = 0;
    OSRdyGrp      = 0;                                /* Clear the ready list */
    for (i = 0; i < OS_RDY_TBL_SIZE; i++) 
    {
        OSRdyTbl[i] = 0;
    }
    OSPrioCur     = 0;
    OSPrioHighRdy = 0;
        
    OSTCBHighRdy  = (OS_TCB *)0;
    OSTCBCur      = (OS_TCB *)0;
    
    ptcb1 = &OSTCBTbl[0];
    ptcb2 = &OSTCBTbl[1];
    for (i = 0; i < (OS_MAX_TASKS + OS_N_SYS_TASKS - 1); i++)
    {
        ptcb1->OSTCBNext = ptcb2;
        ptcb1++;
        ptcb2++;
    }
    ptcb1->OSTCBNext        = (OS_TCB *)0;       /* Last OS_TCB               */
    OSTCBList               = (OS_TCB *)0;       /* TCB lists initializations */
    OSTCBFreeList           = &OSTCBTbl[0];    

    
    
    OSTaskCreate( OS_TaskIdle,
                 &OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE - 1],
                  OS_TASK_IDLE_PRIO);                 /* Create the Idle Task */
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                            INITIALIZE TCB
*
* Description: This function is internal to uC/OS-II and is used to initialize a Task Control Block when
*              a task is created (see OSTaskCreate() and OSTaskCreateExt()).
*
* Arguments  : prio          is the priority of the task being created
*
*              ptos          is a pointer to the task's top-of-stack assuming that the CPU registers
*                            have been placed on the stack.  Note that the top-of-stack corresponds to a
*                            'high' memory location is uint32_t_GROWTH is set to 1 and a 'low' memory
*                            location if uint32_t_GROWTH is set to 0.  Note that stack growth is CPU
*                            specific.
*
* Returns    : OS_ERR_NONE         if the call was successful
*              OS_ERR_TASK_NO_MORE_TCB  if there are no more free TCBs to be allocated and thus, the task cannot
*                                  be created.
*
*********************************************************************************************************
*/

static uint8_t  OS_TCBInit (uint8_t prio, uint32_t *ptos)
{
    OS_TCB    *ptcb;
//#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    uint32_t  cpu_sr = 0;
//#endif

    OS_ENTER_CRITICAL();
        
    ptcb = OSTCBFreeList;                                  /* Get a free TCB from the free TCB list    */
    if (ptcb != (OS_TCB *)0)
    {
        OSTCBFreeList            = ptcb->OSTCBNext;        /* Update pointer to free TCB list          */
        OS_EXIT_CRITICAL();
        ptcb->OSTCBStkPtr        = ptos;                   /* Load Stack pointer in TCB                */
        ptcb->OSTCBPrio          = prio;                   /* Load task priority into TCB              */
        ptcb->OSTCBStat          = OS_STAT_RDY;            /* Task is ready to run                     */
//        ptcb->OSTCBStatPend      = OS_STAT_PEND_OK;        /* Clear pend status                        */
        ptcb->OSTCBDly           = 0;                      /* Task is not delayed                      */

        ptcb->OSTCBY             = (uint8_t)(prio >> 3);          /* Pre-compute X, Y, BitX and BitY     */
        ptcb->OSTCBX             = (uint8_t)(prio & 0x07);
        ptcb->OSTCBBitY          = (uint8_t)(1 << ptcb->OSTCBY);
        ptcb->OSTCBBitX          = (uint8_t)(1 << ptcb->OSTCBX);
        
        OS_ENTER_CRITICAL();                
 
        OSTCBPrioTbl[prio] = ptcb;
        ptcb->OSTCBNext    = OSTCBList;                    /* Link into TCB chain                      */
        ptcb->OSTCBPrev    = (OS_TCB *)0;
        if (OSTCBList != (OS_TCB *)0) {
            OSTCBList->OSTCBPrev = ptcb;
        }
        OSTCBList               = ptcb;
        OSRdyGrp               |= ptcb->OSTCBBitY;         /* Make task ready to run                   */
        OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
                
        OS_EXIT_CRITICAL();
        return (OS_ERR_NONE);
    }
    OS_EXIT_CRITICAL();
    return (OS_ERR_TASK_NO_MORE_TCB);
}


static uint32_t *OSTaskStkInit (void (*task)(void), uint32_t *ptos)
{
    uint32_t *stk;
        
    stk       = ptos;                     /* Load stack pointer               */
                        /* Registers stacked as if [auto-saved] on exception  */
    *(stk)   = (uint32_t)0x01000000L;     /* xPSR                             */
    *(--stk) = (uint32_t)task;            /* Entry Point                      */
    *(--stk) = (uint32_t)0xFFFFFFFEL;     /* R14 (LR)                         */
    *(--stk) = (uint32_t)0x12121212L;     /* R12                              */
    *(--stk) = (uint32_t)0x03030303L;     /* R3                               */
    *(--stk) = (uint32_t)0x02020202L;     /* R2                               */
    *(--stk) = (uint32_t)0x01010101L;     /* R1                               */
    *(--stk) = (uint32_t)0x00000000L;     /* R0 : argument                    */
                        /* Remaining registers saved on process stack         */
    *(--stk) = (uint32_t)0x11111111L;     /* R11                              */
    *(--stk) = (uint32_t)0x10101010L;     /* R10                              */
    *(--stk) = (uint32_t)0x09090909L;     /* R9                               */
    *(--stk) = (uint32_t)0x08080808L;     /* R8                               */
    *(--stk) = (uint32_t)0x07070707L;     /* R7                               */
    *(--stk) = (uint32_t)0x06060606L;     /* R6                               */
    *(--stk) = (uint32_t)0x05050505L;     /* R5                               */
    *(--stk) = (uint32_t)0x04040404L;     /* R4                               */

    return (stk);
}


uint8_t  OSTaskCreate (void (*task)(void), uint32_t *ptos, uint8_t prio)
{
    uint32_t    *psp;
    uint8_t      err;
    
    uint32_t  cpu_sr = 0;
    
 
        
    if (OSTCBPrioTbl[prio] == (OS_TCB *)0) { /* Make sure task doesn't already exist at this priority  */
        OSTCBPrioTbl[prio] = OS_TCB_RESERVED;/* Reserve the priority to prevent others from doing ...  */
                                             /* ... the same thing until task is created.              */
        OS_EXIT_CRITICAL();
        psp = OSTaskStkInit(task, ptos);                      /* Initialize the task's stack         */                
        err = OS_TCBInit(prio, psp);                
        return (err);
    }

    OS_EXIT_CRITICAL();
    return (OS_ERR_PRIO_EXIST);
}


void  OSStart (void)
{
    OS_SchedNew();                               /* Find highest priority's task priority */
    OSPrioCur     = OSPrioHighRdy;
    OSTCBHighRdy  = OSTCBPrioTbl[OSPrioHighRdy]; /* Point to highest priority task ready to run    */
    OSTCBCur      = OSTCBHighRdy;
    OSStartHighRdy();
}


/********************* (C) COPYRIGHT 2015 Windy Albert **************************** END OF FILE ********/
