#ifdef BUILD_LK
#else
    #include <linux/string.h>
    #if defined(BUILD_UBOOT)
        #include <asm/arch/mt_gpio.h>
    #else
        #include <mach/mt_gpio.h>
    #endif
#endif
#include "lcm_drv.h"


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  						(480)
#define FRAME_HEIGHT 						(800)

#define REGFLAG_DELAY             				0xAB
#define REGFLAG_END_OF_TABLE      				0xAA   // END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define LCM_ID       (0x55)
#define LCM_ID1       (0xC1)
#define LCM_ID2       (0x80)

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
     //#Enable Page1
     {0xF0, 5,{0x55,0xAA,0x52,0x08,0x01}},

     //#AVDD Manual
     {0xB6, 3,{0x34,0x34,0x34}},
     {0xB0, 3,{0x09,0x09,0x09}},

     //#AVEE Manual, 6V
     {0xB7, 3,{0x24,0x24,0x24}},
     {0xB1, 3,{0x09,0x09,0x09}},

     //#VGH Clamp Enable
     {0xB9, 3,{0x24,0x24,0x24}},
     {0xB3, 3,{0x05,0x05,0x05}},
     {0xBF, 1,{0x01}},

     //#LVGL
     {0xBA, 3,{0x34,0x34,0x34}},

     //#VGL_REG(VGLO)
     {0xB5, 3,{0x0B,0x0B,0x0B}},

     //#VGMP/VGSP
     {0xBC, 3,{0x00,0xA3,0x00}},

     //#VGMN/VGSN
     {0xBD, 3,{0x00,0xA3,0x00}},

     //#VCOM  
     {0xBE, 2,{0x00,0x3a}},

     //#Gamma R+ Setting
     {0xD1, 52,{0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,
		0x00,0xD2,0x00,0xF6,0x01,0x27,0x01,0x4E,0x01,0x8C,
		0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,
		0x02,0xBC,0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,
		0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,0x03,0xB9,
		0x03,0xC1}},
     //#Gamma G+ Setting
     {0xD2, 52,{0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,
		0x00,0xD2,0x00,0xF6,0x01,0x27,0x01,0x4E,0x01,0x8C,
		0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,
		0x02,0xBC,0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,
		0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,0x03,0xB9,
		0x03,0xC1}},

     //#Gamma B+ Setting
     {0xD3, 52,{0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,
		0x00,0xD2,0x00,0xF6,0x01,0x27,0x01,0x4E,0x01,0x8C,
		0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,
		0x02,0xBC,0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,
		0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,0x03,0xB9,
		0x03,0xC1}},

     //#Gamma R- Setting
     {0xD4, 52,{0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,
		0x00,0xD2,0x00,0xF6,0x01,0x27,0x01,0x4E,0x01,0x8C,
		0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,
		0x02,0xBC,0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,
		0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,0x03,0xB9,
		0x03,0xC1}},

     //#Gamma G- Setting
     {0xD5, 52,{0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,
		0x00,0xD2,0x00,0xF6,0x01,0x27,0x01,0x4E,0x01,0x8C,
		0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,
		0x02,0xBC,0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,
		0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,0x03,0xB9,
		0x03,0xC1}},

     //#Gamma B- Setting
     {0xD6, 52,{0x00,0x37,0x00,0x52,0x00,0x7B,0x00,0x99,0x00,0xB1,
		0x00,0xD2,0x00,0xF6,0x01,0x27,0x01,0x4E,0x01,0x8C,
		0x01,0xBE,0x02,0x0B,0x02,0x48,0x02,0x4A,0x02,0x7E,
		0x02,0xBC,0x02,0xE1,0x03,0x10,0x03,0x31,0x03,0x5A,
		0x03,0x73,0x03,0x94,0x03,0x9F,0x03,0xB3,0x03,0xB9,
		0x03,0xC1}},

     //#LV2 Page 0 enable
     {0xF0, 5,{0x55,0xAA,0x52,0x08,0x00}},

     //#SDT
     {0xB6, 1,{0x0A}},

     //#Gate EQ control
     {0xB7, 2,{0x00,0x00}},

     //#Source EQ control (Mode 2)
     {0xB8, 4,{0x01,0x05,0x05,0x05}},
     {0xBA, 1,{0x01}},

     //#Inversion mode  (2-dot))
     {0xBC, 3,{0x00,0x00,0x00}},

     //#BOE's Setting
     {0xCC, 3,{0x03,0x00,0x00}},

     //#Display timing
     {0xBD, 5,{0x01,0x4d,0x07,0x31,0x00}},
     {0xB1, 2,{0xFc,0x00}},
     {0xB5, 1,{0x50}},//50 480*800    6B 480*854

     {0x36, 1,{0x00}},
     {0x3a, 1,{0x77}},

	{0x11,	1,	{0x00}},
     {REGFLAG_DELAY, 150, {}},                                
	{0x29,	1,	{0x00}},
     {REGFLAG_DELAY, 50, {}},

     {REGFLAG_END_OF_TABLE, 0x00, {}}
/*{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},

{0xB0,3,{0x09,0x09,0x09}},

{0xB6,3,{0x34,0x34,0x34}},

{0xB1,3,{0x09,0x09,0x09}},

{0xB7,3,{0x24,0x24,0x24}},

{0xB3,3,{0x05,0x05,0x05}},  

{0xB9,3,{0x24,0x24,0x24}}, 

{0xbf,1,{0x01}},

{0xB5,3,{0x0b,0x0b,0x0b}},

{0xBA,3,{0x34,0x34,0x34}},

{0xBC,3,{0x00,0xA3,0X00}}, 

{0xBD,3,{0x00,0xA3,0x00}},
 
{0xBE,2,{0x00,0x36}},
           
{0xD1,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},  
{0xD2,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},
{0xD3,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},
{0xD4,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},
{0xD5,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},
{0xD6,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},

{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},

{0xB6,1,{0x0a}},
 
{0xB7,2,{0x00,0x00}},

{0xB8,4,{0x01,0x05,0x05,0x05}},

{0xBa,1,{0x01}},

{0xBC,3,{0x00,0x00,0x00}},

{0xBD,5,{0x01,0x84,0x07,0x31,0x00}},

{0xBE,5,{0x01,0x84,0x07,0x31,0x00}},

{0xBF,5,{0x01,0x84,0x07,0x31,0x00}},

{0xCC,3,{0x03,0x00,0x00}},

{0xB1,2,{0xFC,0x00}},

{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},

{0xB4,1,{0x10}},

{0xFF,4,{0xAA,0x55,0x25,0x01}},

{0xF9,11,{0x0A,0x00,0x0F,0x23,0x37,0x4B,0x5F,0x73,0x87,0x9B,0xAF}},
//{0x35,1,{0x00}},
//{0x36,1,{0x00}},
//{0x3A,1,{0x77}},

{0x11,1,{0x00}},
{REGFLAG_DELAY, 120, {}},
{0x29,1,{0x00}},
{REGFLAG_DELAY, 10, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}*/
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    
    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 10, {}},
    
    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;
    
    for(i = 0; i < count; i++) {
        unsigned cmd;
        cmd = table[i].cmd;
        
        switch (cmd) {
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
            
            case REGFLAG_END_OF_TABLE :
                break;
            
            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);				
            
                }
        }
    
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));
	params->type   = LCM_TYPE_DSI;
	params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;
    
    // enable tearing-free
	params->dbi.te_mode 			= LCM_DBI_TE_MODE_DISABLED;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
    
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
    
    // DSI
    /* Command mode setting */
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine. 
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST; 
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;
	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.vertical_sync_active				= 2;
		params->dsi.vertical_backporch					= 12;// 8;2
		params->dsi.vertical_frontporch					= 16;// 8;2
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		params->dsi.horizontal_sync_active				= 4;//6
		params->dsi.horizontal_backporch				= 40;//37
		params->dsi.horizontal_frontporch				= 40;//37
//	params->dsi.horizontal_blanking_pixel		       = 60;
	params->dsi.horizontal_active_pixel		       = FRAME_WIDTH;

	// Bit rate calculation
	params->dsi.PLL_CLOCK=195;
}



static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(120);

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
    unsigned int data_array[16];

    data_array[0] = 0x00280500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(20);
    
    data_array[0] = 0x00100500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(120);
    
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(120);
}


static void lcm_resume(void)
{
    lcm_init();
}
         

static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
    unsigned int x0 = x;
    unsigned int y0 = y;
    unsigned int x1 = x0 + width - 1;
    unsigned int y1 = y0 + height - 1;
    
    unsigned char x0_MSB = ((x0>>8)&0xFF);
    unsigned char x0_LSB = (x0&0xFF);
    unsigned char x1_MSB = ((x1>>8)&0xFF);
    unsigned char x1_LSB = (x1&0xFF);
    unsigned char y0_MSB = ((y0>>8)&0xFF);
    unsigned char y0_LSB = (y0&0xFF);
    unsigned char y1_MSB = ((y1>>8)&0xFF);
    unsigned char y1_LSB = (y1&0xFF);
    
    unsigned int data_array[16];
    

    data_array[0]= 0x00053902;
    data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
    data_array[2]= (x1_LSB);
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]= 0x00053902;
    data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
    data_array[2]= (y1_LSB);
    dsi_set_cmdq(data_array, 3, 1);
    
    data_array[0]= 0x002c3909;
    dsi_set_cmdq(data_array, 1, 0);
}

static unsigned int lcm_compare_id(void)
{
    unsigned int id = 0, id2 = 0;
    unsigned char buffer[2];
    unsigned int data_array[16];
    

    SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);	
    
    /*	
    data_array[0] = 0x00110500;		// Sleep Out
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(120);
    */
    
    //*************Enable CMD2 Page1  *******************//
    data_array[0]=0x00063902;
    data_array[1]=0x52AA55F0;
    data_array[2]=0x00000108;
    dsi_set_cmdq(data_array, 3, 1);
    MDELAY(10); 
    
    data_array[0] = 0x00023700;// read id return two byte,version and id
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(10); 
    
    read_reg_v2(0xC5, buffer, 2);
    id = buffer[0]; //we only need ID
    id2= buffer[1]; //we test buffer 1
    
    return (LCM_ID == id && 0x10 == id2)?1:0;
}


// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hct_nt35510_dsi_vdo_wvga_boe = 
{
    .name			= "hct_nt35510_dsi_vdo_wvga_boe",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id    = lcm_compare_id,
//    .update         = lcm_update
};

