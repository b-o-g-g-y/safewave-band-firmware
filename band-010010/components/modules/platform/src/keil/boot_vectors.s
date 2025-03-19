;/*****************************************************************************
; * @file:    startup_MPS_CM3.s
; * @purpose: CMSIS Cortex-M3 Core Device Startup File 
; *           for the ARM 'Microcontroller Prototyping System' 
; * @version: V1.01
; * @date:    19. Aug. 2009
; *------- <<< Use Configuration Wizard in Context Menu >>> ------------------
; *
; * Copyright (C) 2008-2009 ARM Limited. All rights reserved.
; * ARM Limited (ARM) is supplying this software for use with Cortex-M3 
; * processor based microcontrollers.  This file can be freely distributed 
; * within development tools that are supporting such ARM based processors. 
; *
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; *****************************************************************************/

; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

                IMPORT Reset_Handler
                IMPORT rwble_isr_ram
                    
                PRESERVE8
                THUMB

; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                
__initial_sp    EQU     0x1100c000

                DCD     __initial_sp ;
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     MemManage_Handler         ; MPU Fault Handler
                DCD     BusFault_Handler          ; Bus Fault Handler
                DCD     UsageFault_Handler        ; Usage Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler
                    
                DCD     rwble_isr_ram             ; 0
                DCD     dma_isr                   ; 1
                DCD     timer0_isr                ; 2
                DCD     timer1_isr                ; 3
                DCD     i2c0_isr                  ; 4
                DCD     i2c1_isr                  ; 5
                DCD     spim0_isr                 ; 6
                DCD     spim1_isr                 ; 7
                DCD     spis_isr                  ; 8
                DCD     uart0_isr                 ; 9
                DCD     uart1_isr                 ; 10
                DCD     pdm_isr                   ; 11
                DCD     0                         ; 12
                DCD     0                         ; 13
                DCD     exti_isr                  ; 14
                DCD     pmu_isr                   ; 15
                DCD     0                         ; 16
                DCD     0                         ; 17
                DCD     pwm_isr                   ; 18
                DCD     usb_isr                   ; 19
                DCD     0                         ; 20
                DCD     lcd_isr                   ; 21
                DCD     0                         ; 22
                DCD     i2s_isr                   ; 23
                AREA    |.text|, CODE, READONLY

; Dummy Exception Handlers (infinite loops which can be modified)
Default_Handler PROC
                EXPORT  NMI_Handler             [WEAK]
                EXPORT  HardFault_Handler       [WEAK]
                EXPORT  MemManage_Handler       [WEAK]
                EXPORT  BusFault_Handler        [WEAK]
                EXPORT  UsageFault_Handler      [WEAK]
                EXPORT  DebugMon_Handler        [WEAK]
                EXPORT  PendSV_Handler          [WEAK]
                EXPORT  SysTick_Handler         [WEAK]
                EXPORT  dma_isr                 [WEAK]
                EXPORT  timer0_isr              [WEAK]
                EXPORT  timer1_isr              [WEAK]
                EXPORT  i2c0_isr                [WEAK]
                EXPORT  i2c1_isr                [WEAK]
                EXPORT  spim0_isr               [WEAK]
                EXPORT  spim1_isr               [WEAK]
                EXPORT  spis_isr                [WEAK]
                EXPORT  uart0_isr               [WEAK]
                EXPORT  uart1_isr               [WEAK]
                EXPORT  uart0_isr               [WEAK]
                EXPORT  uart1_isr               [WEAK]
                EXPORT  exti_isr                [WEAK]
                EXPORT  pmu_isr                 [WEAK]
                EXPORT  pwm_isr                 [WEAK]
                EXPORT  usb_isr                 [WEAK]
                EXPORT  lcd_isr                 [WEAK]
                EXPORT  i2s_isr                 [WEAK]
                EXPORT  pdm_isr                 [WEAK]
NMI_Handler
HardFault_Handler
MemManage_Handler
BusFault_Handler
UsageFault_Handler
DebugMon_Handler
PendSV_Handler
SysTick_Handler
dma_isr
timer0_isr
timer1_isr
i2c0_isr
i2c1_isr
spim0_isr
spim1_isr
spis_isr
uart0_isr
uart1_isr
exti_isr
pmu_isr
pwm_isr
usb_isr
lcd_isr
i2s_isr
pdm_isr
                B       .
                ENDP
                
SVC_Handler     PROC    
                EXPORT  SVC_Handler                 [WEAK]
                IMPORT  svc_exception_handler  
                
                PUSH    {LR}
                LDR     R0, [SP, #0x1C]                
                LDR     R1, =svc_exception_handler                
                LDR     R1, [R1, #0]
                BLX     R1                
                STR     R0, [SP, #0x1C]                
                POP     {PC}
                ENDP

                END

