
#include "driver_dma.h"
#include "driver_system.h"
#include "driver_if8080.h"
#include "driver_gpio.h"

#define LCD_RELEASE_RESET()         gpio_write_pin(GPIO_E, GPIO_PIN_0, GPIO_PIN_SET)
#define LCD_SET_RESET()             gpio_write_pin(GPIO_E, GPIO_PIN_0, GPIO_PIN_CLEAR)

static DMA_LLI_InitTypeDef Link_Channel[20];
static DMA_HandleTypeDef jd9854_DMA_Channel;
static dma_LinkParameter_t LinkParameter;
static volatile bool dma_transfer_done = true;
static void (*dma_trans_done_callback)(void) = NULL;

static void jd9854_init_io(void)
{
    GPIO_InitTypeDef GPIO_Handle;

    GPIO_Handle.Pin       = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_Handle.Mode      = GPIO_MODE_AF_PP;
    GPIO_Handle.Pull      = GPIO_PULLDOWN;
    GPIO_Handle.Alternate = GPIO_FUNCTION_D;
    gpio_init(GPIO_B, &GPIO_Handle);

    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_LCD_WRX); // 8080_WR
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_LCD_RDX); // 8080_RD
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_LCD_CSX); // 8080_CS
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_LCD_DCX); // 8080_DC
    
    // reset
    __SYSTEM_GPIO_CLK_ENABLE();
    GPIO_Handle.Pin       = GPIO_PIN_0;
    GPIO_Handle.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init(GPIO_E, &GPIO_Handle);
}

static void jd9854_init_8080(void)
{
    str_LCDParam_t str_LCDParam;
    
    __SYSTEM_IF8080_CLK_ENABLE();
    __SYSTEM_IF8080_CLK_SELECT_48M();
    
    str_LCDParam.LcdMode = MODE_8080;
    str_LCDParam.DataBusSelect = DATA_BUS_8_BIT;
    str_LCDParam.ReadClock = RDCLK_DIV_8;
    str_LCDParam.WriteClock = WDCLK_DIV_1; //WDCLK_DIV_2

    if8080_init(str_LCDParam);
    IF8080->DATA_CFG.DATA_TRANS_SEQ_0 = 1;
    IF8080->DATA_CFG.DATA_TRANS_SEQ_1 = 0;
    IF8080->DATA_CFG.DATA_TRANS_SEQ_2 = 3;
    IF8080->DATA_CFG.DATA_TRANS_SEQ_3 = 2;
}

static void jd9854_init_dma(void)
{
    __SYSTEM_DMA_CLK_ENABLE();
    
    jd9854_DMA_Channel.Channel = DMA_Channel0;
    jd9854_DMA_Channel.Init.Data_Flow        = DMA_M2P_DMAC;
    jd9854_DMA_Channel.Init.Request_ID       = 0;
    jd9854_DMA_Channel.Init.Source_Inc       = DMA_ADDR_INC_INC;
    jd9854_DMA_Channel.Init.Desination_Inc   = DMA_ADDR_INC_NO_CHANGE;
    jd9854_DMA_Channel.Init.Source_Width     = DMA_TRANSFER_WIDTH_32;
    jd9854_DMA_Channel.Init.Desination_Width = DMA_TRANSFER_WIDTH_32;

    dma_init(&jd9854_DMA_Channel);
    
    NVIC_EnableIRQ(DMA_IRQn);
}

static void jd9854_write_cmd(uint8_t cmd)
{
    if8080_cs_set();
	if8080_write_cmd(cmd);
	if8080_cs_release();
}

static void jd9854_write_param(uint8_t data)
{
    if8080_cs_set();
	if8080_write_param(data);
	if8080_cs_release();
}

void jd9854_init(void)
{
    jd9854_init_io();
    jd9854_init_8080();
    jd9854_init_dma();
    
    LCD_RELEASE_RESET();
	co_delay_100us(500);
	LCD_SET_RESET();
	co_delay_100us(1000);
	LCD_RELEASE_RESET();
	co_delay_100us(500);
    
    jd9854_write_cmd(0xDF); //Password
    jd9854_write_param(0x58);
    jd9854_write_param(0x58);
    jd9854_write_param(0xB0);

    //---------------- PAGE0 --------------
    jd9854_write_cmd(0xDE);
    jd9854_write_param(0x00);

    //VCOM_SET
    jd9854_write_cmd(0xB2);
    jd9854_write_param(0x01);
    jd9854_write_param(0x10); //VCOM:-0.25V

    //Gamma_Set
    jd9854_write_cmd(0xB7);
    jd9854_write_param(0x10); //VGMP = +5 V 0x12C
    jd9854_write_param(0x2C);
    jd9854_write_param(0x00); //VGSP = +0.0V
    jd9854_write_param(0x10); //VGMN = -5V 0x12C
    jd9854_write_param(0x2C);
    jd9854_write_param(0x00); //VGSN = -0.0V

    //DCDC_SEL
    jd9854_write_cmd(0xBB);
    jd9854_write_param(0x01); //VGH = AVDD+VCI = 5.5V+3.1V= 8.6V ;VGL = -1*VGH = -8.6V; AVDD = 2xVCI = 3.1V*2 = 6.2V
    jd9854_write_param(0x1A); //AVDD_S = +5.5V (0x1A) ; AVEE = -1xAVDD_S = -5.5V
    jd9854_write_param(0x44);
    jd9854_write_param(0x44);
    jd9854_write_param(0x33);
    jd9854_write_param(0x33);

    //GATE_POWER
    jd9854_write_cmd(0xCF);
    jd9854_write_param(0x20); //VGHO = +8V
    jd9854_write_param(0x50); //VGLO = -8V

    //SET_R_GAMMA
    jd9854_write_cmd(0xC8);
    jd9854_write_param(0x7F);
    jd9854_write_param(0x4B);
    jd9854_write_param(0x33);
    jd9854_write_param(0x25);
    jd9854_write_param(0x1E);
    jd9854_write_param(0x0E);
    jd9854_write_param(0x13);
    jd9854_write_param(0x01);
    jd9854_write_param(0x1E);
    jd9854_write_param(0x21);
    jd9854_write_param(0x23);
    jd9854_write_param(0x44);
    jd9854_write_param(0x34);
    jd9854_write_param(0x3E);
    jd9854_write_param(0x33);
    jd9854_write_param(0x33);
    jd9854_write_param(0x2A);
    jd9854_write_param(0x1C);
    jd9854_write_param(0x01);
    jd9854_write_param(0x7F);
    jd9854_write_param(0x4B);
    jd9854_write_param(0x33);
    jd9854_write_param(0x25);
    jd9854_write_param(0x1E);
    jd9854_write_param(0x0E);
    jd9854_write_param(0x13);
    jd9854_write_param(0x01);
    jd9854_write_param(0x1E);
    jd9854_write_param(0x21);
    jd9854_write_param(0x23);
    jd9854_write_param(0x44);
    jd9854_write_param(0x34);
    jd9854_write_param(0x3E);
    jd9854_write_param(0x33);
    jd9854_write_param(0x33);
    jd9854_write_param(0x2A);
    jd9854_write_param(0x1C);
    jd9854_write_param(0x01);

    //-----------------------------
    // SET page4 TCON & GIP
    //------------------------------
    jd9854_write_cmd(0xDE);
    jd9854_write_param(0x04); // page4

    //SETSTBA
    jd9854_write_cmd(0xB2);
    jd9854_write_param(0x14); //GAP = 1 ;SAP= 4
    jd9854_write_param(0x14);

    //SETRGBCYC1
    jd9854_write_cmd(0xB8);
    jd9854_write_param(0x74); //-	NEQ	PEQ[1:0] -	RGB_INV_NP[2:0]
    jd9854_write_param(0x44); //-	RGB_INV_PI[2:0] -	RGB_INV_I[2:0]
    jd9854_write_param(0x00); //RGB_N_T2[11:8],RGB_N_T1[11:8]
    jd9854_write_param(0x01); //RGB_N_T1[7:0],
    jd9854_write_param(0x01); //RGB_N_T2[7:0],
    jd9854_write_param(0x00); //RGB_N_T4[11:8],RGB_N_T3[11:8]
    jd9854_write_param(0x01); //RGB_N_T3[7:0],
    jd9854_write_param(0x01); //RGB_N_T4[7:0],
    jd9854_write_param(0x00); //RGB_N_T6[11:8],RGB_N_T5[11:8]
    jd9854_write_param(0x27); //RGB_N_T5[7:0],
    jd9854_write_param(0x99); //RGB_N_T6[7:0],
    jd9854_write_param(0x10); //RGB_N_T8[11:8],RGB_N_T7[11:8]
    jd9854_write_param(0xA3); //RGB_N_T7[7:0],
    jd9854_write_param(0x15); //RGB_N_T8[7:0],
    jd9854_write_param(0x11); //RGB_N_T10[11:8],RGB_N_T9[11:8]
    jd9854_write_param(0x1F); //RGB_N_T9[7:0],
    jd9854_write_param(0x91); //RGB_N_T10[7:0],
    jd9854_write_param(0x21); //RGB_N_T12[11:8],RGB_N_T11[11:8]
    jd9854_write_param(0x9B); //RGB_N_T11[7:0],
    jd9854_write_param(0x0D); //RGB_N_T12[7:0],
    jd9854_write_param(0x22); //RGB_N_T14[11:8],RGB_N_T13[11:8]
    jd9854_write_param(0x17); //RGB_N_T13[7:0],
    jd9854_write_param(0x89); //RGB_N_T14[7:0],
    jd9854_write_param(0x32); //RGB_N_T16[11:8],RGB_N_T15[11:8]
    jd9854_write_param(0x93); //RGB_N_T15[7:0],
    jd9854_write_param(0x05); //RGB_N_T16[7:0],
    jd9854_write_param(0x00); //RGB_N_T18[11:8],RGB_N_T17[11:8]
    jd9854_write_param(0x00); //RGB_N_T17[7:0],
    jd9854_write_param(0x00); //RGB_N_T18[7:0],

    //SETRGBCYC2
    jd9854_write_cmd(0xB9);
    jd9854_write_param(0x00); //-,ENJDT,RGB_JDT2[2:0],ENP_LINE_INV,ENP_FRM_SEL[1:0],
    jd9854_write_param(0x22); //RGB_N_T20[11:8],RGB_N_T19[11:8],
    jd9854_write_param(0x08); //RGB_N_T19[7:0],
    jd9854_write_param(0x3A); //RGB_N_T20[7:0],
    jd9854_write_param(0x22); //RGB_N_T22[11:8],RGB_N_T21[11:8],
    jd9854_write_param(0x4B); //RGB_N_T21[7:0],
    jd9854_write_param(0x7D); //RGB_N_T22[7:0],
    jd9854_write_param(0x22); //RGB_N_T24[11:8],RGB_N_T23[11:8],
    jd9854_write_param(0x8D); //RGB_N_T23[7:0],
    jd9854_write_param(0xBF); //RGB_N_T24[7:0],
    jd9854_write_param(0x32); //RGB_N_T26[11:8],RGB_N_T25[11:8],
    jd9854_write_param(0xD0); //RGB_N_T25[7:0],
    jd9854_write_param(0x02); //RGB_N_T26[7:0],
    jd9854_write_param(0x33); //RGB_N_T28[11:8],RGB_N_T27[11:8],
    jd9854_write_param(0x12); //RGB_N_T27[7:0],
    jd9854_write_param(0x44); //RGB_N_T28[7:0],
    jd9854_write_param(0x00); //-,-,-,-,RGB_N_TA1[11:8],
    jd9854_write_param(0x0A); //RGB_N_TA1[7:0],
    jd9854_write_param(0x00); //RGB_N_TA3[11:8],RGB_N_TA2[11:8],
    jd9854_write_param(0x0A); //RGB_N_TA2[7:0],
    jd9854_write_param(0x0A); //RGB_N_TA3[7:0],
    jd9854_write_param(0x00); //RGB_N_TA5[11:8],RGB_N_TA4[11:8],
    jd9854_write_param(0x0A); //RGB_N_TA4[7:0],
    jd9854_write_param(0x0A); //RGB_N_TA5[7:0],
    jd9854_write_param(0x00); //RGB_N_TA7[11:8],RGB_N_TA6[11:8],
    jd9854_write_param(0x0A); //RGB_N_TA6[7:0],
    jd9854_write_param(0x0A); //RGB_N_TA7[7:0],

    //SETRGBCYC3
    jd9854_write_cmd(0xBA);
    jd9854_write_param(0x00); //-	-	-	-	-	-	-	-
    jd9854_write_param(0x00); //RGB_N_TA9[11:8],RGB_N_TA8[11:8]
    jd9854_write_param(0x07); //RGB_N_TA8[7:0],
    jd9854_write_param(0x07); //RGB_N_TA9[7:0],
    jd9854_write_param(0x00); //RGB_N_TA11[11:8],RGB_N_TA10[11:8]
    jd9854_write_param(0x07); //RGB_N_TA10[7:0],
    jd9854_write_param(0x07); //RGB_N_TA11[7:0],
    jd9854_write_param(0x00); //RGB_N_TA13[11:8],RGB_N_TA12[11:8]
    jd9854_write_param(0x07); //RGB_N_TA12[7:0],
    jd9854_write_param(0x07); //RGB_N_TA13[7:0],
    jd9854_write_param(0x00); //RGB_N_TC[11:8],RGB_N_TB[11:8]
    jd9854_write_param(0x01); //RGB_N_TB[7:0],
    jd9854_write_param(0x01); //RGB_N_TC[7:0],
    jd9854_write_param(0x00); //RGB_N_TE[11:8],RGB_N_TD[11:8]
    jd9854_write_param(0x0A); //RGB_N_TD[7:0],
    jd9854_write_param(0x01); //RGB_N_TE[7:0],
    jd9854_write_param(0x00); //-	-	-	-	RGB_N_TF[11:8]
    jd9854_write_param(0x01); //RGB_N_TF[7:0],
    jd9854_write_param(0x30); //RGB_CHGEN_OFF[11:8],RGB_CHGEN_ON[11:8]
    jd9854_write_param(0x0A); //RGB_CHGEN_ON[7:0],
    jd9854_write_param(0x0E); //RGB_CHGEN_OFF[7:0],
    jd9854_write_param(0x30); //RES_MUX_OFF[11:8],RES_MUX_ON[11:8]
    jd9854_write_param(0x01); //RES_MUX_ON[7:0],
    jd9854_write_param(0x0E); //RES_MUX_OFF[7:0],
    jd9854_write_param(0x00); //-	-	-	L2_COND1_INV[12:8],
    jd9854_write_param(0x00); //-	-	-	L2_COND0_INV[12:8],
    jd9854_write_param(0x00); //L2_COND0_INV[7:0],
    jd9854_write_param(0x00); //L2_COND1_INV[7:0],

    //SETRGBCYC4
    jd9854_write_cmd(0xBB);
    jd9854_write_param(0x10);
    jd9854_write_param(0x66);
    jd9854_write_param(0x66);
    jd9854_write_param(0xA0);
    jd9854_write_param(0x80);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x60);
    jd9854_write_param(0x00);
    jd9854_write_param(0xC0);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x40);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);

    //SET_TCON
    jd9854_write_cmd(0xBC);
    jd9854_write_param(0x18); //1  MUX_SEL =1:6 ,RSO = 320H
    jd9854_write_param(0x09); //2  LN_NO_MUL2 = 1:Gate line number=LN[10:0] ,LN[10:8] = 1
    jd9854_write_param(0x81); //3  LN[7:0] = 385
    jd9854_write_param(0x03); //4  PANEL[2:0] = dancing type 2
    jd9854_write_param(0x00); //5  VFP[11:8],SLT[11:8]
    jd9854_write_param(0xC4); //6  SLT[7:0] = 1/(60*(385+8+8))/4OSC(18.818MHZ) = 196d = 0x0C4 //41.66us
    jd9854_write_param(0x08); //7  VFP[7:0] = 8
    jd9854_write_param(0x00); //8  HBP[11:8], VBP[11:8]
    jd9854_write_param(0x07); //9  VBP[7:0]
    jd9854_write_param(0x2C); //10 HBP[7:0]
    jd9854_write_param(0x00); //11 VFP_I[11:8],SLT_I[11:8]
    jd9854_write_param(0xC4); //12 SLT_I[7:0]
    jd9854_write_param(0x08); //13 VFP_I[7:0]
    jd9854_write_param(0x00); //14 HBP_I[11:8],VBP_I[11:8]
    jd9854_write_param(0x07); //15 VBP_I[7:0]
    jd9854_write_param(0x2C); //16 HBP_I[7:0]
    jd9854_write_param(0x82); //17 HBP_NCK[3:0],HFP_NCK[3:0]
    jd9854_write_param(0x00); //18 TCON_OPT1[15:8]
    jd9854_write_param(0x03); //19 TCON_OPT1[7:0]
    jd9854_write_param(0x00); //20 VFP_PI[11:8],SLT_PI[11:8]
    jd9854_write_param(0xC4); //21 SLT_PI[7:0]
    jd9854_write_param(0x08); //22 VFP_PI[7:0]
    jd9854_write_param(0x00); //23 HBP_PI[11:8],VBP_PI[11:8]
    jd9854_write_param(0x07); //24 VBP_PI[7:0]
    jd9854_write_param(0x2C); //25 HBP_PI[7:0]

    //-------------------GIP----------------------
    //SET_GIP_EQ
    jd9854_write_cmd(0xC4);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x02);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x02);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x02);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);

    //SET_GIP_L
    jd9854_write_cmd(0xC5);
    jd9854_write_param(0x00); //DUMMY
    jd9854_write_param(0x1F); //0
    jd9854_write_param(0x1F); //1
    jd9854_write_param(0x1F); //2
    jd9854_write_param(0x1F); //3
    jd9854_write_param(0x1F); //4   SB :VGL
    jd9854_write_param(0x1E); //5   GAS
    jd9854_write_param(0x02); //6   STVB
    jd9854_write_param(0xC5); //7   CKV2
    jd9854_write_param(0xC7); //8   CKV4
    jd9854_write_param(0xC9); //9   CKV6
    jd9854_write_param(0xCB); //10  CKV8
    jd9854_write_param(0xCD); //11  CKV10
    jd9854_write_param(0xE1); //12  CKH2
    jd9854_write_param(0xE3); //13  CKH4
    jd9854_write_param(0xE5); //14  CKH6
    jd9854_write_param(0x1F); //15
    jd9854_write_param(0x1F); //16
    jd9854_write_param(0x1F); //17
    jd9854_write_param(0x1F); //18
    jd9854_write_param(0x1F); //19
    jd9854_write_param(0x1F); //20
    jd9854_write_param(0x1F); //21
    jd9854_write_param(0x1F); //22
    jd9854_write_param(0x1F); //23
    jd9854_write_param(0x1F); //24
    jd9854_write_param(0x1F); //25

    //SET_GIP_R
    jd9854_write_cmd(0xC6);
    jd9854_write_param(0x00); //DUMMY
    jd9854_write_param(0x1F); //0
    jd9854_write_param(0x1F); //1
    jd9854_write_param(0x1F); //2
    jd9854_write_param(0x1F); //3
    jd9854_write_param(0x1F); //4  ASB :VGL
    jd9854_write_param(0xDF); //5  BGAS
    jd9854_write_param(0x00); //6  STVF
    jd9854_write_param(0xC4); //7  CKV1
    jd9854_write_param(0xC6); //8  CKV3
    jd9854_write_param(0xC8); //9  CKV5
    jd9854_write_param(0xCA); //10 CKV7
    jd9854_write_param(0xCC); //11 CKV9
    jd9854_write_param(0xE0); //12 CKH1
    jd9854_write_param(0xE2); //13 CKH3
    jd9854_write_param(0xE4); //14 CKH5
    jd9854_write_param(0x1F); //15
    jd9854_write_param(0x1F); //16
    jd9854_write_param(0x1F); //17
    jd9854_write_param(0x1F); //18
    jd9854_write_param(0x1F); //19
    jd9854_write_param(0x1F); //20
    jd9854_write_param(0x1F); //21
    jd9854_write_param(0x1F); //22
    jd9854_write_param(0x1F); //23
    jd9854_write_param(0x1F); //24
    jd9854_write_param(0x1F); //25

    //SET_GIP_L_GS
    jd9854_write_cmd(0xC7);
    jd9854_write_param(0x00); //DUMMY
    jd9854_write_param(0x1F); //0
    jd9854_write_param(0x1F); //1
    jd9854_write_param(0x1F); //2
    jd9854_write_param(0x1F); //3
    jd9854_write_param(0x1F); //4   SB :VGL
    jd9854_write_param(0xDE); //5   GAS
    jd9854_write_param(0x00); //6   STVB
    jd9854_write_param(0xC7); //7   CKV2
    jd9854_write_param(0xC5); //8   CKV4
    jd9854_write_param(0xCD); //9   CKV6
    jd9854_write_param(0xCB); //10  CKV8
    jd9854_write_param(0xC9); //11  CKV10
    jd9854_write_param(0x21); //12  CKH2
    jd9854_write_param(0x23); //13  CKH4
    jd9854_write_param(0x25); //14  CKH6
    jd9854_write_param(0x1F); //15
    jd9854_write_param(0x1F); //16
    jd9854_write_param(0x1F); //17
    jd9854_write_param(0x1F); //18
    jd9854_write_param(0x1F); //19
    jd9854_write_param(0x1F); //20
    jd9854_write_param(0x1F); //21
    jd9854_write_param(0x1F); //22
    jd9854_write_param(0x1F); //23
    jd9854_write_param(0x1F); //24
    jd9854_write_param(0x1F); //25

    //SET_GIP_R_GS
    jd9854_write_cmd(0xC8);
    jd9854_write_param(0x00); //DUMMY
    jd9854_write_param(0x1F); //0
    jd9854_write_param(0x1F); //1
    jd9854_write_param(0x1F); //2
    jd9854_write_param(0x1F); //3
    jd9854_write_param(0x1F); //4  ASB :VGL
    jd9854_write_param(0x1F); //5  BGAS
    jd9854_write_param(0x02); //6  STVF
    jd9854_write_param(0xC8); //7  CKV1
    jd9854_write_param(0xC6); //8  CKV3
    jd9854_write_param(0xC4); //9  CKV5
    jd9854_write_param(0xCC); //10 CKV7
    jd9854_write_param(0xCA); //11 CKV9
    jd9854_write_param(0x20); //12 CKH1
    jd9854_write_param(0x22); //13 CKH3
    jd9854_write_param(0x24); //14 CKH5
    jd9854_write_param(0x1F); //15
    jd9854_write_param(0x1F); //16
    jd9854_write_param(0x1F); //17
    jd9854_write_param(0x1F); //18
    jd9854_write_param(0x1F); //19
    jd9854_write_param(0x1F); //20
    jd9854_write_param(0x1F); //21
    jd9854_write_param(0x1F); //22
    jd9854_write_param(0x1F); //23
    jd9854_write_param(0x1F); //24
    jd9854_write_param(0x1F); //25

    //SETGIP1
    jd9854_write_cmd(0xC9);
    jd9854_write_param(0x00); //0
    jd9854_write_param(0x00); //1
    jd9854_write_param(0x00); //2
    jd9854_write_param(0x00); //3
    jd9854_write_param(0x00); //4    R:ASB :VGL L:SB :VGL
    jd9854_write_param(0x20); //5    R:BGAS     L:GAS
    jd9854_write_param(0x00); //6    R:STVF     L:STVB	:TBD STV1 or ETV
    jd9854_write_param(0x30); //7    R:CKV1     L:CKV2
    jd9854_write_param(0x30); //8    R:CKV3     L:CKV4
    jd9854_write_param(0x30); //9    R:CKV5     L:CKV6
    jd9854_write_param(0x30); //10   R:CKV7     L:CKV8
    jd9854_write_param(0x30); //11   R:CKV9     L:CKV10
    jd9854_write_param(0x30); //12   R:CKH1     L:CKH2
    jd9854_write_param(0x30); //13   R:CKH3     L:CKH4
    jd9854_write_param(0x30); //14   R:CKH5     L:CKH6
    jd9854_write_param(0x00); //15
    jd9854_write_param(0x00); //16
    jd9854_write_param(0x00); //17
    jd9854_write_param(0x00); //18
    jd9854_write_param(0x00); //19
    jd9854_write_param(0x00); //20
    jd9854_write_param(0x00); //21
    jd9854_write_param(0x00); //22
    jd9854_write_param(0x00); //23
    jd9854_write_param(0x00); //24
    jd9854_write_param(0x00); //25

    //SETGIP2
    jd9854_write_cmd(0xCB);
    jd9854_write_param(0x01); //1  INIT_PORCH
    jd9854_write_param(0x10); //2  INIT_W
    jd9854_write_param(0x00); //3
    jd9854_write_param(0x10); //4
    jd9854_write_param(0x07); //5  STV_S0
    jd9854_write_param(0x89); //6
    jd9854_write_param(0x00); //7
    jd9854_write_param(0x0A); //8
    jd9854_write_param(0x80); //9  STV_NUM = 1 , STV_S1
    jd9854_write_param(0x00); //10
    jd9854_write_param(0x00); //11 STV1/0_W
    jd9854_write_param(0x00); //12 STV3/2_W
    jd9854_write_param(0x00); //13
    jd9854_write_param(0x07); //14
    jd9854_write_param(0x00); //15
    jd9854_write_param(0x00); //16
    jd9854_write_param(0x00); //17
    jd9854_write_param(0x20); //18
    jd9854_write_param(0x23); //19
    jd9854_write_param(0x90); //20 CKV_W
    jd9854_write_param(0x00); //21
    jd9854_write_param(0x08); //22 CKV_S0
    jd9854_write_param(0x05); //23 CKV0_DUM[7:0]
    jd9854_write_param(0x00); //24
    jd9854_write_param(0x00); //25
    jd9854_write_param(0x05); //26
    jd9854_write_param(0x10); //27
    jd9854_write_param(0x01); //28 //END_W
    jd9854_write_param(0x04); //29
    jd9854_write_param(0x06); //30
    jd9854_write_param(0x10); //31
    jd9854_write_param(0x10); //32

    //SET_GIP_ONOFF
    jd9854_write_cmd(0xD1);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);
    jd9854_write_param(0x03);
    jd9854_write_param(0x60);
    jd9854_write_param(0x30);
    jd9854_write_param(0x03);
    jd9854_write_param(0x18);
    jd9854_write_param(0x30); //CKV0_OFF[11:8]
    jd9854_write_param(0x25); //CKV0_ON[7:0]
    jd9854_write_param(0x0E); //CKV0_OFF[7:0]
    jd9854_write_param(0x30);
    jd9854_write_param(0x03);
    jd9854_write_param(0x18);
    jd9854_write_param(0x30);
    jd9854_write_param(0x03);
    jd9854_write_param(0x18);
    jd9854_write_param(0x30);
    jd9854_write_param(0x03);
    jd9854_write_param(0x18);

    //SET_GIP_ONOFF_WB
    jd9854_write_cmd(0xD2);
    jd9854_write_param(0x00);
    jd9854_write_param(0x30); //STV_OFF[11:8]
    jd9854_write_param(0x25); //STV_ON[7:0]
    jd9854_write_param(0x0E); //STV_OFF[7:0]
    jd9854_write_param(0x32);
    jd9854_write_param(0xBC);
    jd9854_write_param(0x20);
    jd9854_write_param(0x32);
    jd9854_write_param(0xBC);
    jd9854_write_param(0x20);
    jd9854_write_param(0x32);
    jd9854_write_param(0xBC);
    jd9854_write_param(0x20);
    jd9854_write_param(0x32);
    jd9854_write_param(0xBC);
    jd9854_write_param(0x20);
    jd9854_write_param(0x30);
    jd9854_write_param(0x10);
    jd9854_write_param(0x20);
    jd9854_write_param(0x30);
    jd9854_write_param(0x10);
    jd9854_write_param(0x20);
    jd9854_write_param(0x30);
    jd9854_write_param(0x10);
    jd9854_write_param(0x20);
    jd9854_write_param(0x30);
    jd9854_write_param(0x10);
    jd9854_write_param(0x20);

    //SETGIP8_CKH1 CKH_ON/OFF_CKH0-CKH7_odd
    jd9854_write_cmd(0xD4);
    jd9854_write_param(0x00);
    jd9854_write_param(0x00); //CKH_T2_ODD[11:8],CKH_T1_ODD[11:8]
    jd9854_write_param(0x03); //CKH_T1_ODD[7:0],
    jd9854_write_param(0x14); //CKH_T2_ODD[7:0],
    jd9854_write_param(0x00); //CKH_T4_ODD[11:8],CKH_T3_ODD[11:8]
    jd9854_write_param(0x03); //CKH_T3_ODD[7:0],
    jd9854_write_param(0x20); //CKH_T4_ODD[7:0],
    jd9854_write_param(0x00); //CKH_T6_ODD[11:8],CKH_T5_ODD[11:8]
    jd9854_write_param(0x27); //CKH_T5_ODD[7:0],
    jd9854_write_param(0x99); //CKH_T6_ODD[7:0],
    jd9854_write_param(0x10); //CKH_T8_ODD[11:8],CKH_T7_ODD[11:8]
    jd9854_write_param(0xA3); //CKH_T7_ODD[7:0],
    jd9854_write_param(0x15); //CKH_T8_ODD[7:0],
    jd9854_write_param(0x11); //CKH_T10_ODD[11:8],CKH_T9_ODD[11:8]
    jd9854_write_param(0x1F); //CKH_T9_ODD[7:0],
    jd9854_write_param(0x91); //CKH_T10_ODD[7:0],
    jd9854_write_param(0x21); //CKH_T12_ODD[11:8],CKH_T11_ODD[11:8]
    jd9854_write_param(0x9B); //CKH_T11_ODD[7:0],
    jd9854_write_param(0x0D); //CKH_T12_ODD[7:0],
    jd9854_write_param(0x22); //CKH_T14_ODD[11:8],CKH_T13_ODD[11:8]
    jd9854_write_param(0x17); //CKH_T13_ODD[7:0],
    jd9854_write_param(0x89); //CKH_T14_ODD[7:0],
    jd9854_write_param(0x32); //CKH_T16_ODD[11:8],CKH_T15_ODD[11:8]
    jd9854_write_param(0x93); //CKH_T15_ODD[7:0],
    jd9854_write_param(0x05); //CKH_T16_ODD[7:0],
    jd9854_write_param(0x00); //CKH_T18_ODD[11:8],CKH_T17_ODD[11:8]
    jd9854_write_param(0x00); //CKH_T17_ODD[7:0],
    jd9854_write_param(0x00); //CKH_T18_ODD[7:0],
    jd9854_write_param(0x00); //CKH_T20_ODD[11:8],CKH_T19_ODD[11:8]
    jd9854_write_param(0x00); //CKH_T19_ODD[7:0],
    jd9854_write_param(0x00); //CKH_T20_ODD[7:0],

    ///-----------------------------------------------------------------------------------------
    //---------------- PAGE0 --------------
    jd9854_write_cmd(0xDE);
    jd9854_write_param(0x00);
    // RAM_CTRL
    jd9854_write_cmd(0xD7);
    jd9854_write_param(0x20); //GM=1;RP=0;RM=0;DM=00
    jd9854_write_param(0x00);
    jd9854_write_param(0x00);

    //---------------- PAGE1 --------------
    jd9854_write_cmd(0xDE);
    jd9854_write_param(0x01);

    ////MCMD_CTRL
    jd9854_write_cmd(0xCA);
    jd9854_write_param(0x01);

    //---------------- PAGE2 --------------
    jd9854_write_cmd(0xDE);
    jd9854_write_param(0x02);

    //OSC DIV
    jd9854_write_cmd(0xC5);
    jd9854_write_param(0x04); //FPS 60HZ (0x03) /OSCD 18.818MHZ

    //---------------- PAGE0 --------------
    jd9854_write_cmd(0xDE);
    jd9854_write_param(0x00);

    //Color Pixel Format
    jd9854_write_cmd(0x3A);
    jd9854_write_param(0x55); //07:888

    //TE ON
    jd9854_write_cmd(0x35);
    jd9854_write_param(0x00);

    //SLP OUT
    jd9854_write_cmd(0x11); // SLPOUT
    co_delay_100us(1000);

    //The host could send 1st image at this time

    //DISP ON
    jd9854_write_cmd(0x29); // DSPON
    co_delay_100us(500);
}

void jd9854_set_window(uint16_t x_s, uint16_t x_e, uint16_t y_s, uint16_t y_e)
{
    jd9854_write_cmd(0x2A);
	jd9854_write_param(x_s>>8);
	jd9854_write_param(x_s&0xff);
	jd9854_write_param(x_e>>8);
	jd9854_write_param(x_e&0xff);

	jd9854_write_cmd(0x2B);
	jd9854_write_param(y_s>>8);
	jd9854_write_param(y_s&0xff);
	jd9854_write_param(y_e>>8);
	jd9854_write_param(y_e&0xff);

	jd9854_write_cmd(0x2c);
}

void jd9854_display_wait_transfer_done(void)
{
    while(dma_transfer_done == false);
}

void jd9854_display(uint32_t pixel_count, uint16_t *data, void(*callback)(void))
{
    uint32_t i;
    uint32_t total_count = pixel_count / 2;   // accoding source width
    uint8_t link_count = total_count / 4000;
    
    if(dma_transfer_done == false)
    {
        return;
    }
    else
    {
        dma_transfer_done = false;
    }
    
    dma_trans_done_callback = callback;
    
    if(total_count % 4000)
    {
        link_count++;
    }
    
    for (i = 0; i < link_count; i++)
    {
        uint8_t all_set = (total_count <= 4000);
        
        LinkParameter.SrcAddr          = (uint32_t)&data[i * 8000];
        LinkParameter.DstAddr          = (uint32_t)&IF8080->TX_FIFO;
        if(all_set)
        {
            LinkParameter.NextLink     = 0;
        }
        else
        {
            LinkParameter.NextLink     = (uint32_t)&Link_Channel[i + 1];
        }
        LinkParameter.Data_Flow        = DMA_M2P_DMAC;
        LinkParameter.Request_ID       = jd9854_DMA_Channel.Init.Request_ID;
        LinkParameter.Source_Inc       = DMA_ADDR_INC_INC;
        LinkParameter.Desination_Inc   = DMA_ADDR_INC_NO_CHANGE;
        LinkParameter.Source_Width     = DMA_TRANSFER_WIDTH_32;
        LinkParameter.Desination_Width = DMA_TRANSFER_WIDTH_32;
        LinkParameter.Burst_Len        = DMA_BURST_LEN_4;
        LinkParameter.Size             = all_set ? (total_count) : 4000;
        LinkParameter.gather_enable    = 0;
        LinkParameter.scatter_enable   = 0;
        total_count -= 4000;

        dma_linked_list_init(&Link_Channel[i], &LinkParameter);
    }

    __DMA_GATHER_FUNC_DISABLE(jd9854_DMA_Channel.Channel);
    IF8080->DATA_WR_LEN.LEN = pixel_count * 2;
    if8080_cs_set();
    dma_linked_list_start_IT(Link_Channel, &LinkParameter, jd9854_DMA_Channel.Channel);
}

void jd9854_display_gather(uint32_t pixel_count, uint16_t *data, uint16_t interval, uint16_t count)
{
#define DMA_TRANSFER_SIZE       4000
    uint32_t total_count = pixel_count / 2;   // accoding source width
    uint16_t line_count_in_single_list = DMA_TRANSFER_SIZE/(count/2);
    uint16_t src_width_count_in_single_list = line_count_in_single_list*(count/2);
    uint8_t link_count, i;
    
    if(dma_transfer_done == false)
    {
        return;
    }
    else
    {
        dma_transfer_done = false;
    }
    
    link_count = total_count / src_width_count_in_single_list;
    if(total_count % src_width_count_in_single_list)
    {
        link_count++;
    }
    
    for (i = 0; i < link_count; i++)
    {
        uint8_t all_set = (total_count <= src_width_count_in_single_list);
        
        LinkParameter.SrcAddr          = (uint32_t)&data[i * interval * line_count_in_single_list];
        LinkParameter.DstAddr          = (uint32_t)&IF8080->TX_FIFO;
        if(all_set)
        {
            LinkParameter.NextLink     = 0;
        }
        else
        {
            LinkParameter.NextLink     = (uint32_t)&Link_Channel[i + 1];
        }
        LinkParameter.Data_Flow        = DMA_M2P_DMAC;
        LinkParameter.Request_ID       = jd9854_DMA_Channel.Init.Request_ID;
        LinkParameter.Source_Inc       = DMA_ADDR_INC_INC;
        LinkParameter.Desination_Inc   = DMA_ADDR_INC_NO_CHANGE;
        LinkParameter.Source_Width     = DMA_TRANSFER_WIDTH_32;
        LinkParameter.Desination_Width = DMA_TRANSFER_WIDTH_32;
        LinkParameter.Burst_Len        = DMA_BURST_LEN_4;
        LinkParameter.Size             = all_set ? (total_count) : src_width_count_in_single_list;
        LinkParameter.gather_enable    = 1;
        LinkParameter.scatter_enable   = 0;
        total_count -= src_width_count_in_single_list;

        dma_linked_list_init(&Link_Channel[i], &LinkParameter);
    }

    __DMA_GATHER_FUNC_ENABLE(jd9854_DMA_Channel.Channel);
    __DMA_GATHER_INTERVAL(jd9854_DMA_Channel.Channel, (interval-count)/2);
    __DMA_GATHER_COUNT(jd9854_DMA_Channel.Channel, count/2);
    IF8080->DATA_WR_LEN.LEN = pixel_count * 2;
    if8080_cs_set();
    dma_linked_list_start_IT(Link_Channel, &LinkParameter, jd9854_DMA_Channel.Channel);
}

__attribute__((section("ram_code"))) void jd9854_dma_isr(void)
{
    void (*callback)();
    
    // CS Release
    if8080_cs_release();

    /* Clear Transfer complete status */
    dma_clear_tfr_Status(jd9854_DMA_Channel.Channel);
    /* channel Transfer complete interrupt disable */
    dma_tfr_interrupt_disable(jd9854_DMA_Channel.Channel);

    dma_transfer_done = true;
    callback = dma_trans_done_callback;
    dma_trans_done_callback = NULL;
    if(callback) {
        callback();
    }
}
