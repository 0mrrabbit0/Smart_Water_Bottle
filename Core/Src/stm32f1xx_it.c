/**
 * @file    stm32f1xx_it.c
 * @brief   Interrupt service routines
 */

#include "main.h"
#include "stm32f1xx_it.h"
#include "FreeRTOS.h"
#include "task.h"

void NMI_Handler(void)
{
    while (1) {
    }
}

/* HardFault stack frame snapshot for debugger inspection */
volatile uint32_t g_fault_r0;
volatile uint32_t g_fault_r1;
volatile uint32_t g_fault_r2;
volatile uint32_t g_fault_r3;
volatile uint32_t g_fault_r12;
volatile uint32_t g_fault_lr;
volatile uint32_t g_fault_pc;
volatile uint32_t g_fault_psr;

void HardFault_Handler_C(uint32_t *stack_frame)
{
    g_fault_r0  = stack_frame[0];
    g_fault_r1  = stack_frame[1];
    g_fault_r2  = stack_frame[2];
    g_fault_r3  = stack_frame[3];
    g_fault_r12 = stack_frame[4];
    g_fault_lr  = stack_frame[5];
    g_fault_pc  = stack_frame[6];
    g_fault_psr = stack_frame[7];

    while (1) {
    }
}

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
/* Keil / ARM Compiler */
__asm void HardFault_Handler(void)
{
    TST   LR, #4
    ITE   EQ
    MRSEQ R0, MSP
    MRSNE R0, PSP
    B     __cpp(HardFault_Handler_C)
}
#elif defined(__GNUC__)
/* GCC */
void HardFault_Handler(void)
{
    __asm volatile(
        "TST   LR, #4       \n"
        "ITE   EQ            \n"
        "MRSEQ R0, MSP       \n"
        "MRSNE R0, PSP       \n"
        "B     HardFault_Handler_C \n"
    );
}
#endif

void MemManage_Handler(void)
{
    while (1) {
    }
}

void BusFault_Handler(void)
{
    while (1) {
    }
}

void UsageFault_Handler(void)
{
    while (1) {
    }
}

void DebugMon_Handler(void)
{
}

/* SVC_Handler and PendSV_Handler are provided by the FreeRTOS port via
 * FreeRTOSConfig.h mappings; do not redefine here. */

void SysTick_Handler(void)
{
    HAL_IncTick();

    extern void xPortSysTickHandler(void);
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
}
