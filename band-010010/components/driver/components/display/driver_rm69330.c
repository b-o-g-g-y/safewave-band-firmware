#include <stdint.h>

#include "driver_rm69330.h"
#include "driver_system.h"
#include "driver_spi.h"
#include "driver_gpio.h"
#include "driver_dma.h"

#define RM69330_SPI_SEL     SPIM1
#define RM69330_SPI_REGS    ((struct spi_reg_t *)RM69330_SPI_SEL)

//__attribute__((section("ram_code"))) static void spi_set_frame_type_for_lcd(uint32_t spi_addr,enum spi_frame_size_t frame_size, enum spi_data_frame_format_t data_format)
//{
//    RM69330_SPI_REGS->spi_ctr0.dfs = frame_size;
//    RM69330_SPI_REGS->spi_ctr0.spi_frf = data_format; 
//}

__attribute__((section("ram_code"))) static void rm69330_write_cmd_with_param(uint32_t spi_addr,uint8_t cmd,uint8_t para)
{
//    uint8_t spi_data[5];
//    while(!spi_tx_fifo_is_empty(spi_addr));
//    spi_data[0]=0x02;
//    spi_data[1]=0x00;
//    spi_data[2]=cmd;
//    spi_data[3]=0x00;
//    spi_data[4]=para;
//    spi_send_data(spi_addr, spi_data, 5);
//    spi_wait_busy_bit(SPIM1);
}

__attribute__((section("ram_code"))) static void rm69330_write_cmd(uint32_t spi_addr,uint8_t cmd)
{
//    uint8_t spi_data[4];
//    while(!spi_tx_fifo_is_empty(spi_addr));
//    spi_data[0]=0x02;
//    spi_data[1]=0x00;
//    spi_data[2]=cmd;
//    spi_data[3]=0x00;
//    spi_send_data(spi_addr, spi_data, 4);
//    spi_wait_busy_bit(SPIM1);
}

__attribute__((section("ram_code"))) static  void rm69330_set_position(uint32_t spi_addr,uint8_t pos_type,uint8_t ps_1, uint8_t ps_0, uint8_t pe_1,uint8_t pe_0)
{
//    uint8_t spi_data[8];
//    while(!spi_tx_fifo_is_empty(spi_addr));
//    spi_data[0]=0x02;
//    spi_data[1]=0x00;
//    spi_data[2]=pos_type;
//    spi_data[3]=0x00;
//    spi_data[4]=ps_1;
//    spi_data[5]=ps_0;
//    spi_data[6]=pe_1;
//    spi_data[7]=pe_0;
//    spi_send_data(spi_addr, spi_data, 8);
//    spi_wait_busy_bit(SPIM1);
}

static void rm69330_init_io(void)
{
    // cnfigure LCD controller pin and init LCD controller
    gpio_set_dir(GPIO_PORT_D, GPIO_BIT_0, GPIO_DIR_OUT);
    gpio_set_dir(GPIO_PORT_D, GPIO_BIT_1, GPIO_DIR_OUT);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_0, PORTD0_FUNC_D0);   // vci_en
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_1, PORTD1_FUNC_D1);   // vci_en
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_2, PORTD2_FUNC_SSIM1_IO0);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_3, PORTD3_FUNC_SSIM1_IO1);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_4, PORTD4_FUNC_SSIM1_IO2);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_5, PORTD5_FUNC_SSIM1_IO3);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_SSIM1_CLK);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_SSIM1_CSN);
//    gpio_portd_write((gpio_portd_read() & 0xfc) | 0x01);
    *(uint32_t *)0x5006000C = (*(uint32_t *)0x5006000C & 0xfc) | 0x01;
    co_delay_100us(100);
//    gpio_portd_write((gpio_portd_read() & 0xfc) | 0x03);
    *(uint32_t *)0x5006000C = (*(uint32_t *)0x5006000C & 0xfc) | 0x03;
    co_delay_100us(100);
}

static void rm69330_init_spi(void)
{
//    struct spi_config_t spi_config;
//    spi_config.spi_address = SPIM1;
//    spi_config.source_spi_clk = SPI_SOURCE_CLK_96MHZ;
//    spi_config.spi_frame_format = SPI_FRAME_FORMAT_MOTOROLA;
//    spi_config.spi_frame_size = SPI_FRAME_SIZE_8BITS;
//    spi_config.data_format = SPI_DATA_FRAME_FORMAT_STD;
//    spi_config.spi_transfer_mode = SPI_TRANSFER_MODE_TX_ONLY;
//    spi_config.spi_spch = 1;
//    spi_config.spi_scpol = 1;
//    spi_config.slave_select_toggle = false;
//    spi_config.trans_type = SPI_TRANS_TYPE_INST_STAND_ADDR_STAND;
//    spi_config.spi_addrL = SPI_ADDRESS_LENGTH_24;
//    spi_config.spi_instL =  SPI_INSTRUCTION_LENGTH_8;
//    spi_config.baud_rate = 4000000;
//    spi_init(&spi_config);

//    RM69330_SPI_REGS->spi_dmacr.tdmae = 1;
//    RM69330_SPI_REGS->spi_dmatdlr.dmatdl = 15;
}

static void rm69330_init_dma(void)
{
    DMA_HandleTypeDef DMA_Channel_0;

    *((uint32_t *)0x50000014) = 1 << 28;
    
    DMA_Channel_0.Channel = DMA_Channel0;
    DMA_Channel_0.Init.Data_Flow        = DMA_M2P_DMAC;
    DMA_Channel_0.Init.Request_ID       = 1;
    DMA_Channel_0.Init.Source_Inc       = DMA_ADDR_INC_INC;
    DMA_Channel_0.Init.Desination_Inc   = DMA_ADDR_INC_NO_CHANGE;
    DMA_Channel_0.Init.Source_Width     = DMA_TRANSFER_WIDTH_32;
    DMA_Channel_0.Init.Desination_Width = DMA_TRANSFER_WIDTH_16;

    dma_init(&DMA_Channel_0);
}

void rm69330_init(void)
{
    uint32_t spi_addr = RM69330_SPI_SEL;

    rm69330_init_io();
    rm69330_init_spi();
    rm69330_init_dma();
    
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x01);
    rm69330_write_cmd_with_param(spi_addr,0x05,0x10);
    rm69330_write_cmd_with_param(spi_addr,0x06,0x62);
    rm69330_write_cmd_with_param(spi_addr,0x0D,0x00);
    rm69330_write_cmd_with_param(spi_addr,0x0E,0x81);//gAVDD Charge Pump 81H-6.2V
    rm69330_write_cmd_with_param(spi_addr,0x0F,0x81);//gAVDD Charge Pump 81H-6.2V idle
    rm69330_write_cmd_with_param(spi_addr,0x10,0x11);//gAVDD=2VCI
    rm69330_write_cmd_with_param(spi_addr,0x11,0x81);//gVCL=-VCI,
    rm69330_write_cmd_with_param(spi_addr,0x12,0x81);
    rm69330_write_cmd_with_param(spi_addr,0x13,0x80);//gVGH Charge Pump
    rm69330_write_cmd_with_param(spi_addr,0x14,0x80);
    rm69330_write_cmd_with_param(spi_addr,0x15,0x81);//gVGL Charge Pump  VCL-AVDD
    rm69330_write_cmd_with_param(spi_addr,0x16,0x81);//gVGL
    rm69330_write_cmd_with_param(spi_addr,0x18,0x66);//gVGHR66=6V
    rm69330_write_cmd_with_param(spi_addr,0x19,0x88);//gVGLR aa=-7V 44=-4v
    rm69330_write_cmd_with_param(spi_addr,0x5B,0x10);//gVREFPN5 Regulator Enable
    rm69330_write_cmd_with_param(spi_addr,0x5C,0x55);//gVPREF5 and VNREF5 output status(default);
    rm69330_write_cmd_with_param(spi_addr,0x62,0x19);//gNormal 19 VREFN=-3V 2e=-5v
    rm69330_write_cmd_with_param(spi_addr,0x63,0x19);//gIdle VREFN
    rm69330_write_cmd_with_param(spi_addr,0x70,0x55);//gSource Sequence 2
    rm69330_write_cmd_with_param(spi_addr,0x74,0x0C);//gOVDD / SD power source control
    rm69330_write_cmd_with_param(spi_addr,0xC5,0x10); //g NOR=IDLE=GAM1 # HBM=GAM2
    rm69330_write_cmd_with_param(spi_addr,0x25,0x03);//gNormal
    rm69330_write_cmd_with_param(spi_addr,0x26,0x80);
    rm69330_write_cmd_with_param(spi_addr,0x27,0x08);
    rm69330_write_cmd_with_param(spi_addr,0x28,0x08);
    rm69330_write_cmd_with_param(spi_addr,0x2A,0x23);  //gIDLE  
    rm69330_write_cmd_with_param(spi_addr,0x2B,0x80);
    rm69330_write_cmd_with_param(spi_addr,0x2D,0x08);//gVBP
    rm69330_write_cmd_with_param(spi_addr,0x2F,0x08);//gVFP
    rm69330_write_cmd_with_param(spi_addr,0x30,0x43);  //g43: 15Hz
    rm69330_write_cmd_with_param(spi_addr,0x66,0x90);  //gIdle interal elvdd,elvss
    rm69330_write_cmd_with_param(spi_addr,0x72,0x1A);  //gOVDD  4.6V
    rm69330_write_cmd_with_param(spi_addr,0x73,0x13);  //gOVSS  -2.2V
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x01);
    rm69330_write_cmd_with_param(spi_addr,0x6A,0x03);  //gRT4723 OVSS -2.2V
    rm69330_write_cmd_with_param(spi_addr,0x1B,0x00);  //g8:Deep idle 8color 0:HBM 24bit
    //gVSR power saving
    rm69330_write_cmd_with_param(spi_addr,0x1D,0x03);
    rm69330_write_cmd_with_param(spi_addr,0x1E,0x03);
    rm69330_write_cmd_with_param(spi_addr,0x1F,0x0C);   //gzhuankong crosstalk
    rm69330_write_cmd_with_param(spi_addr,0x20,0x03);
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x01);
    rm69330_write_cmd_with_param(spi_addr,0x36,0x00);
    rm69330_write_cmd_with_param(spi_addr,0x6C,0x80);  //gSD_COPEN_OFF
    rm69330_write_cmd_with_param(spi_addr,0x6D,0x19);  //gVGMP VGSP turn off at idle mode
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x04);
    rm69330_write_cmd_with_param(spi_addr,0x63,0x00);
    rm69330_write_cmd_with_param(spi_addr,0x64,0x0E);
    //gGamma
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x02);
    rm69330_write_cmd_with_param(spi_addr,0xA9,0x40);//g6V VGMP
    rm69330_write_cmd_with_param(spi_addr,0xAA,0xB8);//g2.5V VGSP
    rm69330_write_cmd_with_param(spi_addr,0xAB,0x01);//g
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x03);//gpage2
    rm69330_write_cmd_with_param(spi_addr,0xA9,0x40);//g6V VGMP
    rm69330_write_cmd_with_param(spi_addr,0xAA,0x90);//g2.5V VGSP	
    rm69330_write_cmd_with_param(spi_addr,0xAB,0x01);//g
    //gSwitch Timing Control
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x01);
    rm69330_write_cmd_with_param(spi_addr,0x3A,0x00);//g05
    rm69330_write_cmd_with_param(spi_addr,0x3B,0x41);//g00
    rm69330_write_cmd_with_param(spi_addr,0x3D,0x17);//g05
    rm69330_write_cmd_with_param(spi_addr,0x3F,0x42); //g42
    rm69330_write_cmd_with_param(spi_addr,0x40,0x17);//g09
    rm69330_write_cmd_with_param(spi_addr,0x41,0x06);//g06
    rm69330_write_cmd_with_param(spi_addr,0x37,0x0C);//g0C:VGSP NO SWAP #0C
    //gSW MAPPING
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x0C);
    rm69330_write_cmd_with_param(spi_addr,0x07,0x1F);
    rm69330_write_cmd_with_param(spi_addr,0x08,0x2F);
    rm69330_write_cmd_with_param(spi_addr,0x09,0x3F);
    rm69330_write_cmd_with_param(spi_addr,0x0A,0x4F);
    rm69330_write_cmd_with_param(spi_addr,0x0B,0x5F);
    rm69330_write_cmd_with_param(spi_addr,0x0C,0x6F);
    rm69330_write_cmd_with_param(spi_addr,0x0D,0xFF);
    rm69330_write_cmd_with_param(spi_addr,0x0E,0xFF);
    rm69330_write_cmd_with_param(spi_addr,0x0F,0xFF);
    rm69330_write_cmd_with_param(spi_addr,0x10,0xFF);
    //g181006
    //gSW OUTPUT
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x01);
    rm69330_write_cmd_with_param(spi_addr,0x42,0x14);
    rm69330_write_cmd_with_param(spi_addr,0x43,0x41);
    rm69330_write_cmd_with_param(spi_addr,0x44,0x25);
    rm69330_write_cmd_with_param(spi_addr,0x45,0x52);
    rm69330_write_cmd_with_param(spi_addr,0x46,0x36);
    rm69330_write_cmd_with_param(spi_addr,0x47,0x63);
    rm69330_write_cmd_with_param(spi_addr,0x48,0x41);
    rm69330_write_cmd_with_param(spi_addr,0x49,0x14);
    rm69330_write_cmd_with_param(spi_addr,0x4A,0x52);
    rm69330_write_cmd_with_param(spi_addr,0x4B,0x25);
    rm69330_write_cmd_with_param(spi_addr,0x4C,0x63);
    rm69330_write_cmd_with_param(spi_addr,0x4D,0x36);
    //gSW DATA
    rm69330_write_cmd_with_param(spi_addr,0x4E,0x36);
    rm69330_write_cmd_with_param(spi_addr,0x4F,0x63);
    rm69330_write_cmd_with_param(spi_addr,0x50,0x25);
    rm69330_write_cmd_with_param(spi_addr,0x51,0x52);
    rm69330_write_cmd_with_param(spi_addr,0x52,0x14);
    rm69330_write_cmd_with_param(spi_addr,0x53,0x41);
    rm69330_write_cmd_with_param(spi_addr,0x54,0x63);
    rm69330_write_cmd_with_param(spi_addr,0x55,0x36);
    rm69330_write_cmd_with_param(spi_addr,0x56,0x52);
    rm69330_write_cmd_with_param(spi_addr,0x57,0x25);
    rm69330_write_cmd_with_param(spi_addr,0x58,0x41);
    rm69330_write_cmd_with_param(spi_addr,0x59,0x14);
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x04);
    rm69330_write_cmd_with_param(spi_addr,0x5D,0x01);
    rm69330_write_cmd_with_param(spi_addr,0x75,0x08);
    //gVSR Marping command---L
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x04);
    rm69330_write_cmd_with_param(spi_addr,0x5E,0x9F); //gESTV
    rm69330_write_cmd_with_param(spi_addr,0x5F,0x43); //gECK1 ECK2
    rm69330_write_cmd_with_param(spi_addr,0x60,0xFF);
    rm69330_write_cmd_with_param(spi_addr,0x61,0xFF);
    rm69330_write_cmd_with_param(spi_addr,0x62,0xFF);
    //g   VSR Marping command---R
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x04);
    rm69330_write_cmd_with_param(spi_addr,0x76,0xFF);
    rm69330_write_cmd_with_param(spi_addr,0x77,0xFF);
    rm69330_write_cmd_with_param(spi_addr,0x78,0x10); //gSCK2 STV
    rm69330_write_cmd_with_param(spi_addr,0x79,0xF2); //gSCK1
    rm69330_write_cmd_with_param(spi_addr,0x7A,0xFF);
    //gVSR1-STV
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x04);
    rm69330_write_cmd_with_param(spi_addr,0x00,0x8D);//g9D	
    rm69330_write_cmd_with_param(spi_addr,0x01,0x00);
    rm69330_write_cmd_with_param(spi_addr,0x02,0x00);	
    rm69330_write_cmd_with_param(spi_addr,0x03,0x05);
    rm69330_write_cmd_with_param(spi_addr,0x04,0x00);//g18	
    rm69330_write_cmd_with_param(spi_addr,0x05,0x05);	
    rm69330_write_cmd_with_param(spi_addr,0x06,0x00);//g27
    rm69330_write_cmd_with_param(spi_addr,0x07,0x00);//g63
    rm69330_write_cmd_with_param(spi_addr,0x08,0x00);	
    //gVSR2-SCK2
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x04);
    rm69330_write_cmd_with_param(spi_addr,0x09,0xCC);//gCD	
    rm69330_write_cmd_with_param(spi_addr,0x0A,0x00);
    rm69330_write_cmd_with_param(spi_addr,0x0B,0x02);
    rm69330_write_cmd_with_param(spi_addr,0x0C,0x00);//g01
    rm69330_write_cmd_with_param(spi_addr,0x0D,0x60);//g58	
    rm69330_write_cmd_with_param(spi_addr,0x0E,0x06);
    rm69330_write_cmd_with_param(spi_addr,0x0F,0x2C);//g53	
    rm69330_write_cmd_with_param(spi_addr,0x10,0x53);//gF3
    rm69330_write_cmd_with_param(spi_addr,0x11,0x00);
    //gVSR3-SCK1
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x04);
    rm69330_write_cmd_with_param(spi_addr,0x12,0xCC);//gCD	
    rm69330_write_cmd_with_param(spi_addr,0x13,0x00);	
    rm69330_write_cmd_with_param(spi_addr,0x14,0x02);
    rm69330_write_cmd_with_param(spi_addr,0x15,0x00);//g01	
    rm69330_write_cmd_with_param(spi_addr,0x16,0x60);//g58
    rm69330_write_cmd_with_param(spi_addr,0x17,0x05);
    rm69330_write_cmd_with_param(spi_addr,0x18,0x2C);//g53
    rm69330_write_cmd_with_param(spi_addr,0x19,0x53);//gF3	
    rm69330_write_cmd_with_param(spi_addr,0x1A,0x00);	
    //gVSR4-ECK2
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x04);
    rm69330_write_cmd_with_param(spi_addr,0x1B,0xDC);
    rm69330_write_cmd_with_param(spi_addr,0x1C,0x00);
    rm69330_write_cmd_with_param(spi_addr,0x1D,0x04);
    rm69330_write_cmd_with_param(spi_addr,0x1E,0x02);
    rm69330_write_cmd_with_param(spi_addr,0x1F,0x18);
    rm69330_write_cmd_with_param(spi_addr,0x20,0x06);
    rm69330_write_cmd_with_param(spi_addr,0x21,0x3D);
    rm69330_write_cmd_with_param(spi_addr,0x22,0x75);
    rm69330_write_cmd_with_param(spi_addr,0x23,0x00);
    //gVSR5-ECK1
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x04);	
    rm69330_write_cmd_with_param(spi_addr,0x24,0xDC);
    rm69330_write_cmd_with_param(spi_addr,0x25,0x00);
    rm69330_write_cmd_with_param(spi_addr,0x26,0x04);	
    rm69330_write_cmd_with_param(spi_addr,0x27,0x02);	
    rm69330_write_cmd_with_param(spi_addr,0x28,0x18);
    rm69330_write_cmd_with_param(spi_addr,0x29,0x04);
    rm69330_write_cmd_with_param(spi_addr,0x2A,0x3D);
    rm69330_write_cmd_with_param(spi_addr,0x2B,0x75);
    rm69330_write_cmd_with_param(spi_addr,0x2D,0x00);
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x04);
    rm69330_write_cmd_with_param(spi_addr,0x53,0x8A);
    rm69330_write_cmd_with_param(spi_addr,0x54,0x78);
    rm69330_write_cmd_with_param(spi_addr,0x55,0x08);
    rm69330_write_cmd_with_param(spi_addr,0x56,0x0A);
    rm69330_write_cmd_with_param(spi_addr,0x58,0x2A);
    rm69330_write_cmd_with_param(spi_addr,0x59,0x00);
    rm69330_write_cmd_with_param(spi_addr,0x65,0x02); 
    rm69330_write_cmd_with_param(spi_addr,0x66,0x0A); 
    rm69330_write_cmd_with_param(spi_addr,0x67,0x00);	
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x07);
    rm69330_write_cmd_with_param(spi_addr,0x15,0x04);
    rm69330_write_cmd_with_param(spi_addr,0xFE,0x00);
    rm69330_write_cmd_with_param(spi_addr,0xC4, 0x80);
    rm69330_write_cmd_with_param(spi_addr,0x35,0x00);
    rm69330_write_cmd_with_param(spi_addr,0x51,0xFF);

    rm69330_set_position(spi_addr,0x2a,0x00,0x00,0x01,0x85);
    rm69330_set_position(spi_addr,0x2b,0x00,0x00,0x01,0x85);

    rm69330_write_cmd_with_param(spi_addr,0x3A,0x55);

    rm69330_write_cmd(spi_addr,0x11);
    rm69330_write_cmd(spi_addr,0x29);
}

void rm69330_display(uint16_t x_s, uint16_t x_e, uint16_t y_s, uint16_t y_e, uint16_t *data)
{
    uint16_t header[3];

    //co_printf("%d, %d, %d, %d, 0x%08x\r\n", x_s, x_e, y_s, y_e, data);
    if(x_e != 389) {
        uint16_t offset;
        offset = 389 - x_e;
        offset >>= 2;
        offset <<= 1;
        x_s += offset;
        x_e += offset;
        y_s += offset;
        y_e += offset;
    }

//    rm69330_set_position(RM69330_SPI_SEL, 0x2a, x_s>>8, x_s&0xff, x_e>>8, x_e&0xff);
//    rm69330_set_position(RM69330_SPI_SEL, 0x2b, y_s>>8, y_s&0xff, y_e>>8, y_e&0xff);
//    rm69330_write_cmd(RM69330_SPI_SEL, 0x2c);

//    spi_disable(RM69330_SPI_SEL);
//    spi_set_frame_type_for_lcd(RM69330_SPI_SEL, SPI_FRAME_SIZE_16BITS, SPI_DATA_FRAME_FORMAT_QUAD);
//    spi_enable(RM69330_SPI_SEL);
//    header[0] = 0x32;
//    header[1] = 0x003c;
//    header[2] = 0x00;
//    spi_send_data_16bits(RM69330_SPI_SEL, header, 3);
//    spi_send_data_16bits(RM69330_SPI_SEL, data, (x_e+1-x_s)*(y_e+1-y_s));
//    spi_wait_busy_bit(RM69330_SPI_SEL);

//    spi_disable(RM69330_SPI_SEL);
//    spi_set_frame_type_for_lcd(RM69330_SPI_SEL, SPI_FRAME_SIZE_8BITS, SPI_DATA_FRAME_FORMAT_STD);
//    spi_enable(RM69330_SPI_SEL);
}

