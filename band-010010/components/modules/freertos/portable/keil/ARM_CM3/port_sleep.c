#include "ble_stack.h"
#include "co_printf.h"
#include "driver_pmu.h"
#include "jump_table.h"
#include "driver_uart.h"
#include "core_cm3.h"
#include "sys_utils.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"


#define SLOT_SIZE            625
#define MAX_CLOCK_TIME              ((1L<<28) - 1)
#define CLK_SUB(clock_a, clock_b)     ((uint32_t)(((clock_a) - (clock_b)) & MAX_CLOCK_TIME))
#define CLK_DIFF(clock_a, clock_b)     ( (CLK_SUB((clock_b), (clock_a)) > ((MAX_CLOCK_TIME+1) >> 1)) ?                      \
                          ((int32_t)((-CLK_SUB((clock_a), (clock_b))))) : ((int32_t)((CLK_SUB((clock_b), (clock_a))))) )
                          
#ifndef configSYSTICK_CLOCK_HZ
#define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
/* Ensure the SysTick is clocked at the same frequency as the core. */
#define portNVIC_SYSTICK_CLK_BIT    ( 1UL << 2UL )
#else
/* The way the SysTick is clocked is not modified in case it is not the same
as the core. */
#define portNVIC_SYSTICK_CLK_BIT    ( 0 )
#endif

static volatile uint8_t app_user_rtos_wait_wakeup_end;
static sys_baseband_time_t rtos_sleep_time;
uint32_t uSysTickStopValue;

__attribute__((section("ram_code"))) void freertos_baseband_restore_done(void)
{
#if 1
    sys_baseband_time_t current_time = system_get_baseband_time();
    int32_t diff_time;
    uint32_t ulCompleteTickPeriods, ulReloadValue;

    /* restore systick and pendSV */
    /* unit: us */
    diff_time = CLK_DIFF( (rtos_sleep_time.hs*SLOT_SIZE + rtos_sleep_time.hus)>>1, (current_time.hs*SLOT_SIZE + current_time.hus)>>1 );
    diff_time += 200;

    //co_printf("d:%d,%d,%d\r\n",diff_time,uSysTickStopValue,configSYSTICK_CLOCK_HZ);

    /* unit: sys_tick */
    ulReloadValue = diff_time*(configSYSTICK_CLOCK_HZ/1000000) + uSysTickStopValue;
    ulCompleteTickPeriods = ulReloadValue / (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ);
    ulReloadValue = ulReloadValue % (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ);

    //ulCompleteTickPeriods = diff_time / ( 1000000 / configTICK_RATE_HZ );
    //ulReloadValue = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - system_get_pclk_config() * (diff_time % ( 1000000 / configTICK_RATE_HZ ));

    /* Make PendSV and SysTick the lowest priority interrupts. */
    NVIC_SetPriority(SysTick_IRQn, configKERNEL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(PendSV_IRQn, configKERNEL_INTERRUPT_PRIORITY);

    vTaskStepTick( ulCompleteTickPeriods );

    SysTick->LOAD  = (uint32_t)(ulReloadValue - 1UL);
    SysTick->VAL   = 0UL;
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk   |
                     SysTick_CTRL_ENABLE_Msk;
    SysTick->LOAD = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;

    app_user_rtos_wait_wakeup_end = 1;
    NVIC_SetPriority(BLE_IRQn, 2);
#endif
}
#if 1
__attribute__((section("ram_code"))) void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
    int32_t sleep_duration;
    TickType_t sleep_max_dur;

    /* unit: us */
    sleep_max_dur = ( __jump_table.slp_max_dur >> 1 ) * SLOT_SIZE;
    /* unit: tick */
    sleep_max_dur = pdMS_TO_TICKS( sleep_max_dur / 1000 );
    if(sleep_max_dur < xExpectedIdleTime)
    {
        xExpectedIdleTime = sleep_max_dur;
    }

    /* this value should be taken as sleep delay */
    //if(xExpectedIdleTime > __jump_table.sleep_delay_for_os)
    {
        //xExpectedIdleTime -= __jump_table.sleep_delay_for_os;
    }
    /* unit: half slot */
    sleep_duration = ( ( xExpectedIdleTime * ( 1000000 / configTICK_RATE_HZ ) ) << 1 ) / SLOT_SIZE;
    sleep_duration -=3;
    
    extern volatile int32_t sleep_duration_backup;
    sleep_duration_backup = sleep_duration;
    
    /* Stop the SysTick momentarily.  The time the SysTick is stopped for
    is accounted for as best it can be, but using the tickless mode will
    inevitably result in some tiny drift of the time maintained by the
    kernel with respect to calendar time. */

    /* Enter a critical section but don't use the taskENTER_CRITICAL()
    method as that will mask interrupts that should exit sleep mode. */
    __disable_irq();
    __dsb( portSY_FULL_READ_WRITE );
    __isb( portSY_FULL_READ_WRITE );
    
    uSysTickStopValue = SysTick->VAL;
    rtos_sleep_time = system_get_baseband_time();

    /* If a context switch is pending or a task is waiting for the scheduler
    to be unsuspended then abandon the low power entry. */
    if(( eTaskConfirmSleepModeStatus() == eAbortSleep )
       || ( ble_stack_sleep_time_calc( &sleep_duration ) == false ) )
    {
        //co_printf("N\r\n");
        __enable_irq();
    }
    else
    {
        //bool init_rf = false;
        GLOBAL_INT_DISABLE();
        ble_stack_set_sleep_dur(sleep_duration);
        ble_stack_enter_sleep();

        app_user_rtos_wait_wakeup_end = 0;
        NVIC_SetPriority(BLE_IRQn, 0);
        GLOBAL_INT_RESTORE();
        while(app_user_rtos_wait_wakeup_end == 0);
        ble_stack_schedule_backward();
    }
}
#endif

