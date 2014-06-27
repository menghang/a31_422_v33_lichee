;/*
;**********************************************************************************************************************
;*											        eGon
;*						           the Embedded GO-ON Bootloader System
;*									       eGON arm boot sub-system
;*
;*						  Copyright(C), 2006-2010, SoftWinners Microelectronic Co., Ltd.
;*                                           All Rights Reserved
;*
;* File    : arm_swi
;*
;* By      : Jerry
;*
;* Version : V2.00
;*
;* Date	  :
;*
;* Descript:
;**********************************************************************************************************************
;*/
#include "arm_a8.h"
/*
;*******************************************************************************
;��������: open_sys_int
;����ԭ��: void open_sys_int( void )
;��������: open system irq interrupt
;��ڲ���: void
;�� �� ֵ: void
;��    ע:
;*******************************************************************************
*/
	.globl _sys_int_enable
_sys_int_enable:
	mrs r0, cpsr
	bic r0, r0, #( ARMV7_IRQ_MASK | ARMV7_FIQ_MASK )
	msr cpsr_c, r0
	mov pc, lr


/*
;*******************************************************************************
;��������: close_sys_int
;����ԭ��: void close_sys_int( void )
;��������: close system irq interrupt
;��ڲ���: void
;�� �� ֵ: void
;��    ע:
;*******************************************************************************
*/
	.globl _sys_int_disable
_sys_int_disable:
	mrs r0, cpsr
	orr r0, r0, #( ARMV7_IRQ_MASK | ARMV7_FIQ_MASK )
	msr cpsr_c, r0
	mov pc, lr
