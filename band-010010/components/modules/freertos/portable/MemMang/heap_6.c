#include "FreeRTOS.h"
#include "task.h"

//#include "jump_table.h"
#include "os_mem.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#if( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
#error This file must not be used if configSUPPORT_DYNAMIC_ALLOCATION is 0
#endif

void *pvPortMalloc( size_t xWantedSize )
{
    void *pvReturn;
    //uint8_t mem_block_sel;

    vTaskSuspendAll();
    {
        //if( __jump_table.system_option & SYSTEM_OPTION_RTOS_HEAP_SEL )
        {
            //mem_block_sel = KE_MEM_NON_RETENTION;
        }
        //else
        {
            //mem_block_sel = KE_MEM_ENV;
        }
        pvReturn = os_malloc( xWantedSize );
#if 0
        if( ( uint32_t ) pvReturn & (3) )
        {
            co_printf("pvPortMalloc: return unaligned address.\r\n");
        }
#endif
        traceMALLOC( pvReturn, xWantedSize );
    }
    ( void ) xTaskResumeAll();

#if( configUSE_MALLOC_FAILED_HOOK == 1 )
    {
        if( pvReturn == NULL )
        {
            extern void vApplicationMallocFailedHook( void );
            vApplicationMallocFailedHook();
        }
    }
#endif

    return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree( void *pv )
{
    if( pv )
    {
        vTaskSuspendAll();
        {
            os_free( pv );
            traceFREE( pv, 0 );
        }
        ( void ) xTaskResumeAll();
    }
}

