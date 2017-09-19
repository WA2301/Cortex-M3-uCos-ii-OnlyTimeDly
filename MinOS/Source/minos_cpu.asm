;********************************************************************************************************
;                                               MinOS
;                                         The Real-Time Kernel
;
;                               (c) Copyright 2015-2020, ZH, Windy Albert
;                                          All Rights Reserved
;
;                                           Generic ARM Port
;
; File      : MINOS_CPU.ASM
; Version   : V1.00[From.V2.86]
; By        : Windy Albert & Jean J. Labrosse
;
; For       : ARMv7M Cortex-M3
; Mode      : Thumb2
; Toolchain : RealView Development Suite
;             RealView Microcontroller Development Kit (MDK)
;             ARM Developer Suite (ADS)
;             Keil uVision
;********************************************************************************************************

;********************************************************************************************************
;                                           PUBLIC FUNCTIONS
;********************************************************************************************************

    EXTERN  OSPrioCur					       					; External references
    EXTERN  OSPrioHighRdy
    EXTERN  OSTCBCur
    EXTERN  OSTCBHighRdy

    ;EXPORT  OS_CPU_SR_Save                                      ; Functions declared in this file
    ;EXPORT  OS_CPU_SR_Restore
    ;EXPORT  OSStartHighRdy
    ;EXPORT  OSCtxSw
	
	
	
    EXPORT  PendSV_Handler
	
		
	;IMPORT  OS_CPU_SR_Save                                      ; Functions declared in this file
    ;IMPORT  OS_CPU_SR_Restore
    IMPORT  OSStartHighRdy
    IMPORT  OSCtxSw


;********************************************************************************************************
;                                      CODE GENERATION DIRECTIVES
;********************************************************************************************************

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8


;********************************************************************************************************
;                                         HANDLE PendSV EXCEPTION
;                                     void PendSV_Handler(void)
;
; Note(s) : 1) PendSV is used to cause a context switch.  This is a recommended method for performing
;              context switches with Cortex-M3.  This is because the Cortex-M3 auto-saves half of the
;              processor context on any exception, and restores same on return from exception.  So only
;              saving of R4-R11 is required and fixing up the stack pointers.  Using the PendSV exception
;              this way means that context saving and restoring is identical whether it is initiated from
;              a thread or occurs due to an interrupt or exception.
;
;           2) Pseudo-code is:
;              a) Get the process SP, if 0 then skip (goto d) the saving part (first context switch);
;              b) Save remaining regs r4-r11 on process stack;
;              c) Save the process SP in its TCB, OSTCBCur->OSTCBStkPtr = SP;
;              d) Call OSTaskSwHook();
;              e) Get current high priority, OSPrioCur = OSPrioHighRdy;
;              f) Get current ready thread TCB, OSTCBCur = OSTCBHighRdy;
;              g) Get new process SP from TCB, SP = OSTCBHighRdy->OSTCBStkPtr;
;              h) Restore R4-R11 from new process stack;
;              i) Perform exception return which will restore remaining context.
;
;           3) On entry into PendSV handler:
;              a) The following have been saved on the process stack (by processor):
;                 xPSR, PC, LR, R12, R0-R3
;              b) Processor mode is switched to Handler mode (from Thread mode)
;              c) Stack is Main stack (switched from Process stack)
;              d) OSTCBCur      points to the OS_TCB of the task to suspend
;                 OSTCBHighRdy  points to the OS_TCB of the task to resume
;
;           4) Since PendSV is set to lowest priority in the system (by OSStartHighRdy() above), we
;              know that it will only be run when no other exception or interrupt is active, and
;              therefore safe to assume that context being switched out was using the process stack (PSP).
;********************************************************************************************************

PendSV_Handler
;	//__disable_irq();
    CPSID   I                                                   ; Prevent interruption during context switch
    
	
	;//if(__get_PSP == 0) PendSV_Handler_nosave();//Ϊ0 �����豣��
	MRS     R0, PSP                                             ; PSP is process stack pointer
    CBZ     R0, PendSV_Handler_nosave                     		; Skip register save the first time

    ;//��ջ
	;//R0 = R0 - 0x20; Ų���ߵ�ַ�ݼ�д��              SUBS R0,R1,R2 R0 = R1 - R2
	
    SUBS    R0, R0, #0x20                                       ; Save remaining regs r4-11 on process stack
    STM     R0, {R4-R11}   ;�����Ĵ����ڽ����ж�ʱ��Ӳ���Զ���ջ


    ;//OSTCBCur->OSTCBStkPtr = SP;????
    LDR     R1, =OSTCBCur                                       ; OSTCBCur->OSTCBStkPtr = SP;
    LDR     R1, [R1]
    STR     R0, [R1]                                            ; R0 is SP of process being switched out

	
                                                                ; At this point, entire context of process has been saved
PendSV_Handler_nosave

	; OSPrioCur = OSPrioHighRdy;  Ӧ����ȷ���ο�OSStart������
    LDR     R0, =OSPrioCur                                      ; OSPrioCur = OSPrioHighRdy;
    LDR     R1, =OSPrioHighRdy
    LDRB    R2, [R1]       ;//��ͬ��LDR ��Ϊ ���ֽ�����
    STRB    R2, [R0]


	; OSTCBCur  = OSTCBHighRdy;
    LDR     R0, =OSTCBCur       ;//*OSTCBCur;                  ; OSTCBCur  = OSTCBHighRdy; ��Ӧ���� *OSTCBCur  = *OSTCBHighRdy����
    LDR     R1, =OSTCBHighRdy   ;//*OSTCBHighRdy; ָ�����������
    LDR     R2, [R1]			;//R2<-[R1]  R2�ں�����õ� R1ΪOSTCBHighRdy��[R1]Ϊ�������ջָ��[new process SP]
    STR     R2, [R0]			;//R2->[R0]  R0ΪOSTCBCur��[R0]Ϊ��ǰ�����ջָ��


	;SP = OSTCBHighRdy->OSTCBStkPtr;
    LDR     R0, [R2];//R0<-[R2]    R2Ϊ(OSTCBHighRdy->OSTCBStkPtr)  ; R0 is new process SP; SP = OSTCBHighRdy->OSTCBStkPtr;
    LDM     R0, {R4-R11};�������ڴ�����Ĵ���R0->{R4-R11}       ; Restore r4-11 from new process stack
    ;                    �����Ĵ������жϽ���ʱ��Ӳ������������ǰ��SP�Ѿ��޸ģ����Ե����������������Ӧ�Ĵ���ֵ
	
	;//__set_PSP(OSTCBHighRdy->OSTCBStkPtr+48);????�������й�
	ADDS    R0, R0, #0x20
	MSR     PSP, R0                                             ; Load PSP with new process SP
    
	
	ORR     LR, LR, #0x04                                       ; Ensure exception return uses process stack

;	//__enable_irq();
	CPSIE   I
	
    BX      LR                                                  ; Exception return will restore remaining context

    END
