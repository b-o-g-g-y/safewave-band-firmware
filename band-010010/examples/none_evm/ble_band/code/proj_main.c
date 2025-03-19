/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>

#include "ble_stack.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"

#include "os_timer.h"
#include "os_mem.h"

#include "button.h"
#include "co_printf.h"
#include "sys_utils.h"

#include "ota_service.h"

#include "gap_api.h"

#include "app_config.h"

#include "jump_table.h"
#include "co_log.h"

#include "plf.h"
#include "driver_system.h"
#include "driver_pmu.h"
#include "driver_uart.h"

#include "ble_simple_peripheral.h"
#include "band.h" //
#include "driver_gpio.h"
#include "driver_pwm.h"
#include "driver_adc.h"

#include "driver_timer.h"
#include "driver_efuse.h"
#include "driver_wdt.h"
#include "app.h"

#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL (LOG_LEVEL_INFO)
const char *app_tag = "project";

#define SYSTEM_STACK_SIZE 0x1000
///////////////////////////////////////////////////////////////////////////////////////////////////////////
extern void app_io_init(void);
extern void keyTask(void);

extern void app_pwm_init(void);
extern void app_pwm_deinit(void);
extern void parseAppID(void);
os_timer_t main_task_timer;
os_timer_t sleep_timer;

extern os_timer_t update_timer;
wdt_Init_t wdt_handle;
uint8_t delaySecToSleep;
uint8_t fg_sleep, fg_keywakeup;
extern uint8_t POWERON;
uint8_t fg_timer0;
uint32_t ms_10cnt;

extern uint8_t dev_name[20];
extern uint8_t dev_name_size;

extern void connect_duty_update(void);

uint8_t Pin_value;
GPIO_InitTypeDef GPIO_Handle;

uint32_t temp;

uint32_t efuse[4];
__attribute__((section("stack_section"))) static uint32_t system_stack[SYSTEM_STACK_SIZE / sizeof(uint32_t)];

const struct jump_table_version_t _jump_table_version __attribute__((section("jump_table_3"))) =
	{
		.stack_top_address = &system_stack[SYSTEM_STACK_SIZE / sizeof(uint32_t)],
		.firmware_version = Version,
};

const struct jump_table_image_t _jump_table_image __attribute__((section("jump_table_1"))) =
	{
		.image_type = IMAGE_TYPE_APP,
		.image_size = 0x20000,
};
// 0x20000==128K
// Area-A/B image_size file size is 120K,
// for OTA //the BIN file size must be 4K integer multiple, insufficient area must be filled with FF;
// Remember to add the firmware_version,The firmware_version determines whether the code is executed in area A or B

static void sleep_timer_handler(void *param)
{

	co_printf("sleep");
	app_adc_deinit();
	app_pwm_deinit();
	system_sleep_enable();
	fg_keywakeup = 0;
	fg_sleep = 1;
}

void rf_goto_sleep(void)
{
	if (rf_state == RF_CONNECT)
	{
		gap_disconnect_req(slave_link_conidx);
	}
	else if (rf_state == RF_ADV)
	{
		gap_stop_advertising();
	}
	os_timer_start(&sleep_timer, 2000, false);
}

/*********************************************************************
 * @fn      user_entry_before_sleep_imp
 *
 * @brief   Before system goes to sleep mode, user_entry_before_sleep_imp()
 *          will be called, MCU peripherals can be configured properly before
 *          system goes to sleep, for example, some MCU peripherals need to be
 *          used during the system is in sleep mode.
 *
 * @param   None.
 *
 *
 * @return  None.
 */
__attribute__((section("ram_code"))) void user_entry_before_sleep_imp(void)
{
	// uart_putc_noint_no_wait(UART0, '0');
	co_delay_100us(1);

	pmu_set_pin_to_PMU(GPIO_PORT_A, (1 << GPIO_BIT_0));
	pmu_set_pin_dir(GPIO_PORT_A, (1 << GPIO_BIT_0), GPIO_DIR_IN);
	pmu_set_pin_pull(GPIO_PORT_A, (1 << GPIO_BIT_0), GPIO_PULL_NONE);

	pmu_set_pin_to_PMU(GPIO_PORT_C, (1 << GPIO_BIT_2));
	pmu_set_pin_dir(GPIO_PORT_C, (1 << GPIO_BIT_2), GPIO_DIR_IN);
	pmu_set_pin_pull(GPIO_PORT_C, (1 << GPIO_BIT_2), GPIO_PULL_UP);
	pmu_port_wakeup_func_set(GPIO_PORT_C, GPIO_PIN_2);

	pmu_set_pin_to_PMU(GPIO_PORT_C, (1 << GPIO_BIT_3));
	pmu_set_pin_dir(GPIO_PORT_C, (1 << GPIO_BIT_3), GPIO_DIR_IN);
	pmu_set_pin_pull(GPIO_PORT_C, (1 << GPIO_BIT_3), GPIO_PULL_UP);
	pmu_port_wakeup_func_set(GPIO_PORT_C, GPIO_PIN_3);
	pmu_clear_isr_state(PMU_GPIO_XOR_INT_CLR);

	pmu_calibration_stop();
	wdt_Disable();
}

/*********************************************************************
 * @fn      user_entry_after_sleep_imp
 *
 * @brief   After system wakes up from sleep mode, user_entry_after_sleep_imp()
 *          will be called, MCU peripherals need to be initialized again,
 *          this can be done in user_entry_after_sleep_imp(). MCU peripherals
 *          status will not be kept during the sleep.
 *
 * @param   None.
 *
 *
 * @return  None.
 */
__attribute__((section("ram_code"))) void user_entry_after_sleep_imp(void)
{
	pmu_set_pin_to_CPU(GPIO_PORT_A, (1 << GPIO_BIT_0));
	system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_UART0_RXD);
	system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_UART0_TXD);
	uart_init(UART0, 1152);
	fr_uart_enableIrq(UART0, Uart_irq_erbfi);

	if ((pmu_portc_read() & 0x0C) != 0X0C)
	{
		if (idleCnt)
		{
			idleCnt = 0;
			system_sleep_disable();
			co_printf("Key");
		}
	}
	pmu_set_pin_to_CPU(GPIO_PORT_C, (1 << GPIO_BIT_2));
	pmu_set_pin_to_CPU(GPIO_PORT_C, (1 << GPIO_BIT_3));
	app_io_init();

	NVIC_EnableIRQ(PMU_IRQn);

	pmu_calibration_start(PMU_CALI_SEL_RCLFOSC, LP_RC_CALIB_CNT);
	wdt_Enable();
}

/*********************************************************************
 */

// gap_conn_param_update(slave_link_conidx, 8, 24, 18, 600);

static void main_task_timer_handler(void *param)
{
	if (fg_sleep == 0)
	{
		keyTask();
		charge_task();
		parseAppID();
		if ((fgCharge == 0) && (fgPowerLow == 0))
		{
			motor_task(&motor);
		}
		else
		{
			motor_stop();
			LOCK = 0;
		}
		if ((motor.num == 0) && (fgCharge == 0)) // When vibrating, delayed
		{
			idleCnt++;
		}
		if ((delaySecToSleep) && (fgCharge == 0))
		{
			if (idleCnt == delaySecToSleep * 100)
			{
				idleCnt++;

				// if(rf_state==RF_CONNECT){gap_conn_param_update(slave_link_conidx, 24, 24, 33, 600);}

				//	os_timer_start(&sleep_timer, 3000, false);   GO TO SLEEP
			}
		}
		if ((fg_power_off == 0) && (motor.num == 0)) ////When vibrating, donot detect the battery level
		{
			adc_task();
		}
		os_timer_start(&main_task_timer, 10, false);
	}
}

__attribute__((section("ram_code"))) void main_loop(void)
{
	while (1)
	{
		if (ble_stack_schedule_allow())
		{
			/*user code should be add here*/
			if ((idleCnt == 0) && (fg_sleep))
			{
				idleCnt++;
				fg_sleep = 0;
				system_sleep_disable();
				app_io_init();
				app_pwm_init();
				app_adc_init();
				os_timer_start(&main_task_timer, 10, false);
				co_printf("Wakeup\r\n");
			}
			wdt_Refresh();
			/* schedule internal stack event */
			ble_stack_schedule();
		}

		GLOBAL_INT_DISABLE();
		switch (ble_stack_sleep_check())
		{
		case 2:
		{
			ble_stack_enter_sleep();

			__jump_table.diag_port = 0x00008300;
			*(volatile uint32_t *)0x50000024 = 0xeeeeeeee;
		}
		break;
		default:
			break;
		}
		GLOBAL_INT_RESTORE();

		ble_stack_schedule_backward();
	}
}

/*********************************************************************
 * @fn      proj_init
 *
 * @brief   Main entrancy of user application. This function is called after BLE stack
 *          is initialized, and all the application code will be executed from here.
 *          In that case, application layer initializtion can be startd here.
 *
 * @param   None.
 *
 *
 * @return  None.
 */
void proj_init(void)
{
	LOG_INFO(app_tag, "BLE Peripheral\r\n");
	struct image_info_t *image_info = (struct image_info_t *)(QSPI0_DAC_ADDRESS + 0);
	temp = (image_info->jump_table.image_size + 0x2000);

	// Add a distinctive vibration pattern here
	motor.num = 5;		 // Number of vibrations
	motor.ontime = 100;	 // On time in ms
	motor.offtime = 100; // Off time in ms

	simple_peripheral_init();
	if (fg_power_off == 0)
	{
		os_timer_start(&main_task_timer, 10, false);
		system_sleep_disable();
	}
	GLOBAL_INT_START();
}

/*********************************************************************
 * @fn      user_main
 *
 * @brief   Code to be executed for BLE stack initialization, Power mode
 *          configurations, etc.
 *
 * @param   None.
 *
 *
 * @return  None.
 */
void user_main(void)
{
	app_io_init(); // ��ǰ��ʼ��,��ֹ�����ϵ���;
	/* initialize log module */
	log_init();

	/* initialize PMU module at the beginning of this program */
	pmu_sub_init();					  // set RF tx power
	pmu_ioldo_voltage(PMU_ALDO_2_9V); // LDO-3.0V��̬©��������
	/* set system clock */
	system_set_clock(SYSTEM_CLOCK_SEL);

	/* set local BLE address */
	mac_addr_t mac_addr;
	unsigned char i;
	eFuse_read(efuse);

	mac_addr.addr[0] = efuse[0] & 0xff;
	mac_addr.addr[1] = (efuse[0] >> 8) & 0xff;
	mac_addr.addr[2] = efuse[1] & 0xff;

	i = efuse[0] & 0xff;
	i += (efuse[0] >> 8) & 0xff;
	i += (efuse[0] >> 16) & 0xff;
	i += (efuse[0] >> 24) & 0xff;
	mac_addr.addr[3] = i;

	i = efuse[1] & 0xff;
	i += (efuse[1] >> 8) & 0xff;
	i += (efuse[1] >> 16) & 0xff;
	i += (efuse[1] >> 24) & 0xff;
	mac_addr.addr[4] = i;

	device_name_init();
	/*
	uint8_t  *p;
	static uint8_t jv=0;
	p=dev_name;
	for(i=0;i<dev_name_size;i++)
	{
		jv+=*p;
		p++;
	}
	mac_addr.addr[5] = jv;
	*/
	mac_addr.addr[5] = 0xe0; //

	/*
	i=efuse[2]&0xff;
	i+=(efuse[2]>>8)&0xff;
	i+=(efuse[2]>>16)&0xff;
	i+=(efuse[2]>>24)&0xff;
mac_addr.addr[5] = i;
	*/
	gap_address_set(&mac_addr, BLE_ADDR_TYPE_PRIVATE);

	fg_power_off = 0;
	LOCK = 0;
	idleCnt = 0;
	delaySecToSleep = 30;
	app_pwm_init();
	app_adc_init();
	os_timer_init(&main_task_timer, main_task_timer_handler, NULL);
	os_timer_init(&sleep_timer, sleep_timer_handler, NULL);

	fgPowerLow = 0;
	forceUpdate = 1;

	/* configure ble stack capabilities */
	ble_stack_configure(BLE_STACK_ENABLE_MESH,
						BLE_STACK_ENABLE_CONNECTIONS,
						BLE_STACK_RX_BUFFER_CNT,
						BLE_STACK_RX_BUFFER_SIZE,
						BLE_STACK_TX_BUFFER_CNT,
						BLE_STACK_TX_BUFFER_SIZE,
						BLE_STACK_ADV_BUFFER_SIZE,
						BLE_STACK_RETENTION_RAM_SIZE,
						BLE_STACK_KEY_STORAGE_OFFSET);
	/* initialize ble stack */
	ble_stack_init();
	/* initialize SMP */
	gap_bond_manager_init(BLE_BONDING_INFO_SAVE_ADDR, BLE_REMOTE_SERVICE_SAVE_ADDR, 8, true);

	if (__jump_table.system_option & SYSTEM_OPTION_SLEEP_ENABLE)
	{
		co_printf("\r\n a");
		co_delay_100us(10000); // must keep it, or pressing reset key may block .
		co_printf("\r\n b");
		co_delay_100us(10000);
		co_printf("\r\n c");
		co_delay_100us(10000);
		co_printf("\r\nd");
	}
	proj_init();

	/*
brief description:
First, The watchdog count decrement to 0.
And then the Timeout decrement to 0.
If wdt_Refresh is not called, the system will reset.
*/
	wdt_handle.IRQ_Enable = WDT_IRQ_DISABLE;
	wdt_handle.Timeout = 0xFF;
	wdt_handle.WdtCount = 32000 * 5; // 32K, timeout 5s
	wdt_init(wdt_handle);
	/* enter main loop */
	main_loop();
}
