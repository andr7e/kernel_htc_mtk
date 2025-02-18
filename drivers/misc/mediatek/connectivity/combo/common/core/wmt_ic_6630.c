




#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG         "[WMT-IC]"
#define CFG_IC_MT6630 1

#define MT6630_BRINGUP 0
#include "osal_typedef.h"
#include "wmt_ic.h"
#include "wmt_core.h"
#include "wmt_lib.h"
#include "stp_core.h"


#define DEFAULT_PATCH_FRAG_SIZE (1000)
#define WMT_PATCH_FRAG_1ST (0x1)
#define WMT_PATCH_FRAG_MID (0x2)
#define WMT_PATCH_FRAG_LAST (0x3)

#define CFG_CHECK_WMT_RESULT (1)
#define CFG_WMT_BT_PORT2 (0)	

#define CFG_SET_OPT_REG (0)
#define CFG_WMT_I2S_DBGUART_SUPPORT (0)
#define CFG_SET_OPT_REG_SWLA (0)
#define CFG_SET_OPT_REG_MCUCLK (0)
#define CFG_SET_OPT_REG_MCUIRQ (0)

#define CFG_SUBSYS_COEX_NEED 0

#define CFG_WMT_COREDUMP_ENABLE 0

#define CFG_WMT_MULTI_PATCH (1)

#if CFG_WMT_LTE_COEX_HANDLING
#define CFG_WMT_FILTER_MODE_SETTING (1)
#else
#define CFG_WMT_FILTER_MODE_SETTING (0)
#endif


static UINT8 gFullPatchName[NAME_MAX + 1];
static const WMT_IC_INFO_S *gp_mt6630_info = NULL;
static WMT_PATCH gp_mt6630_patch_info;
static WMT_CO_CLOCK gCoClockEn = WMT_CO_CLOCK_DIS;

static UINT8 WMT_QUERY_BAUD_CMD[] = { 0x01, 0x04, 0x01, 0x00, 0x02 };
static UINT8 WMT_QUERY_BAUD_EVT_115200[] =
    { 0x02, 0x04, 0x06, 0x00, 0x00, 0x02, 0x00, 0xC2, 0x01, 0x00 };
static UINT8 WMT_QUERY_BAUD_EVT_X[] =
    { 0x02, 0x04, 0x06, 0x00, 0x00, 0x02, 0xAA, 0xAA, 0xAA, 0xBB };
static UINT8 WMT_QUERY_STP_CMD[] = { 0x01, 0x04, 0x01, 0x00, 0x04 };
static UINT8 WMT_QUERY_STP_EVT_DEFAULT[] =
    { 0x02, 0x04, 0x06, 0x00, 0x00, 0x04, 0x11, 0x00, 0x00, 0x00 };
static UINT8 WMT_QUERY_STP_EVT_UART[] =
    { 0x02, 0x04, 0x06, 0x00, 0x00, 0x04, 0xDF, 0x0E, 0x68, 0x01 };
static UINT8 WMT_SET_BAUD_CMD_X[] = { 0x01, 0x04, 0x05, 0x00, 0x01, 0xAA, 0xAA, 0xAA, 0xBB };
static UINT8 WMT_SET_BAUD_EVT[] = { 0x02, 0x04, 0x02, 0x00, 0x00, 0x01 };
static UINT8 WMT_SET_WAKEUP_WAKE_CMD_RAW[] = { 0xFF };
static UINT8 WMT_SET_WAKEUP_WAKE_EVT[] = { 0x02, 0x03, 0x02, 0x00, 0x00, 0x03 };
static UINT8 WMT_PATCH_CMD[] = { 0x01, 0x01, 0x00, 0x00, 0x00 };
static UINT8 WMT_PATCH_EVT[] = { 0x02, 0x01, 0x01, 0x00, 0x00 };
static UINT8 WMT_RESET_CMD[] = { 0x01, 0x07, 0x01, 0x00, 0x04 };
static UINT8 WMT_RESET_EVT[] = { 0x02, 0x07, 0x01, 0x00, 0x00 };

#if CFG_WMT_BT_PORT2
static UINT8 WMT_BTP2_CMD[] = { 0x01, 0x10, 0x03, 0x00, 0x01, 0x03, 0x01 };
static UINT8 WMT_BTP2_EVT[] = { 0x02, 0x10, 0x01, 0x00, 0x00 };
#endif

#if CFG_WMT_MULTI_PATCH
static UINT8 WMT_PATCH_ADDRESS_CMD[] =
    { 0x01, 0x08, 0x10, 0x00, 0x01, 0x01, 0x00, 0x01, 0xD4, 0x03, 0x09, 0x02, 0x00, 0x00, 0x00,
0x00, 0xff, 0xff, 0xff, 0xff };
static UINT8 WMT_PATCH_ADDRESS_EVT[] = { 0x02, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x01 };
static UINT8 WMT_PATCH_P_ADDRESS_CMD[] =
    { 0x01, 0x08, 0x10, 0x00, 0x01, 0x01, 0x00, 0x01, 0xfc, 0x08, 0x09, 0x02, 0x00, 0x00, 0x08,
0x00, 0xff, 0xff, 0xff, 0xff };
static UINT8 WMT_PATCH_P_ADDRESS_EVT[] = { 0x02, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x01 };
#endif

static UINT8 WMT_COEX_SETTING_CONFIG_CMD[] = { 0x01, 0x10, 0x02, 0x00, 0x01, 0x00 };
static UINT8 WMT_COEX_SETTING_CONFIG_EVT[] = { 0x02, 0x10, 0x01, 0x00, 0x00 };

#if CFG_SUBSYS_COEX_NEED
static UINT8 WMT_BT_COEX_SETTING_CONFIG_CMD[] = { 0x01, 0x10, 0x0B,
	0x00, 0x02,
	0x00, 0x00, 0x00, 0x00,
	0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0xAA
};
static UINT8 WMT_BT_COEX_SETTING_CONFIG_EVT[] = { 0x02, 0x10, 0x01, 0x00, 0x00 };

static UINT8 WMT_WIFI_COEX_SETTING_CONFIG_CMD[] = { 0x01, 0x10, 0x0C,
	0x00, 0x03,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0xAA
};
static UINT8 WMT_WIFI_COEX_SETTING_CONFIG_EVT[] = { 0x02, 0x10, 0x01, 0x00, 0x00 };

static UINT8 WMT_PTA_COEX_SETTING_CONFIG_CMD[] = { 0x01, 0x10, 0x0A,
	0x00, 0x04,
	0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xEE, 0xFF, 0xFF, 0xFE
};
static UINT8 WMT_PTA_COEX_SETTING_CONFIG_EVT[] = { 0x02, 0x10, 0x01, 0x00, 0x00 };

static UINt8 WMT_MISC_COEX_SETTING_CONFIG_CMD[] = { 0x01, 0x10, 0x09,
	0x00, 0x05,
	0xAA, 0xAA, 0xAA, 0xAA,
	0xBB, 0xBB, 0xBB, 0xBB
};
static UINT8 WMT_MISC_COEX_SETTING_CONFIG_EVT[] = { 0x02, 0x10, 0x01, 0x00, 0x00 };
#endif

static UINT8 WMT_SET_STP_CMD[] = { 0x01, 0x04, 0x05, 0x00, 0x03, 0xDF, 0x0E, 0x68, 0x01 };
static UINT8 WMT_SET_STP_EVT[] = { 0x02, 0x04, 0x02, 0x00, 0x00, 0x03 };
static UINT8 WMT_STRAP_CONF_CMD_FM_COMM[] = { 0x01, 0x05, 0x02, 0x00, 0x02, 0x02 };
static UINT8 WMT_STRAP_CONF_EVT[] = { 0x02, 0x05, 0x02, 0x00, 0x00, 0x02 };

static UINT8 WMT_CORE_DUMP_LEVEL_04_CMD[] =
    { 0x1, 0x0F, 0x07, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static UINT8 WMT_CORE_DUMP_LEVEL_04_EVT[] = { 0x2, 0x0F, 0x01, 0x00, 0x00 };

static UINT8 WMT_CORE_CO_CLOCK_CMD[] = { 0x1, 0x0A, 0x02, 0x00, 0x08, 0x03 };
static UINT8 WMT_CORE_CO_CLOCK_EVT[] = { 0x2, 0x0A, 0x01, 0x00, 0x00 };


#if (MTK_WCN_CMB_MERGE_INTERFACE_SUPPORT)


static UINT8 WMT_SET_DAI_MODE_REG_CMD[] = { 0x01, 0x08, 0x28, 0x00	
	    , 0x01		
	    , 0x01		
	    , 0x00		
	    , 0x03		
	    , 0x6c, 0x50, 0x02, 0x80	
	    , 0x00, 0x00, 0x10, 0x11	
	    , 0x00, 0x00, 0xf0, 0xff	
	    , 0x70, 0x50, 0x02, 0x80	
	    , 0x01, 0x00, 0x00, 0x00	
	    , 0x0f, 0x00, 0x00, 0x00	
	    , 0x00, 0x53, 0x02, 0x80	
	    , 0x04, 0x00, 0x00, 0x00	
	    , 0x04, 0x00, 0x00, 0x00	
};

static UINT8 WMT_SET_DAI_MODE_REG_EVT[] = { 0x02, 0x08, 0x04, 0x00	
	    , 0x00		
	    , 0x00		
	    , 0x00		
	    , 0x03		
};


#endif


#if CFG_SET_OPT_REG_SWLA	
static UINT8 WMT_SET_SWLA_REG_CMD[] = { 0x01, 0x08, 0x1C, 0x00	
	    , 0x01		
	    , 0x01		
	    , 0x00		
	    , 0x02		
	    , 0x10, 0x01, 0x05, 0x80	
	    , 0x10, 0x10, 0x01, 0x00	
	    , 0xF0, 0xF0, 0x0F, 0x00	
	    , 0x40, 0x01, 0x05, 0x80	
	    , 0x00, 0x10, 0x01, 0x00	
	    , 0x00, 0xF0, 0x0F, 0x00	
};

static UINT8 WMT_SET_SWLA_REG_EVT[] = { 0x02, 0x08, 0x04, 0x00	
	    , 0x00		
	    , 0x00		
	    , 0x00		
	    , 0x02		
};
#endif

#if CFG_SET_OPT_REG_MCUCLK	
static UINT8 WMT_SET_MCUCLK_REG_CMD[] = { 0x01, 0x08, (4 + 12 * 4), 0x00	
	    , 0x01		
	    , 0x01		
	    , 0x00		
	    , 0x04		
	    , 0x00, 0x04, 0x00, 0x80	
	    , 0x00, 0x14, 0x00, 0x00	
	    , 0xFF, 0xFF, 0x00, 0x00	
	    , 0x80, 0x01, 0x05, 0x80	
	    , 0x12, 0x13, 0x00, 0x00	
	    , 0xFF, 0xFF, 0x00, 0x00	
	    , 0x00, 0x01, 0x05, 0x80	
	    , 0x00, 0x00, 0x02, 0x00	
	    , 0x00, 0x00, 0x0F, 0x00	
	    , 0x10, 0x01, 0x05, 0x80	
	    , 0x02, 0x00, 0x00, 0x00	
	    , 0x0F, 0x00, 0x00, 0x00	
};

static UINT8 WMT_SET_MCUCLK_REG_EVT[] = { 0x02, 0x08, 0x04, 0x00	
	    , 0x00		
	    , 0x00		
	    , 0x00		
	    , 0x04		
};
#endif

#if CFG_WMT_I2S_DBGUART_SUPPORT	
static UINT8 WMT_SET_DBGUART_REG_CMD[] = { 0x01, 0x08, 0x1C, 0x00	
	    , 0x01		
	    , 0x01		
	    , 0x00		
	    , 0x02		
	    , 0x30, 0x01, 0x05, 0x80	
	    , 0x00, 0x00, 0x00, 0x00	
	    , 0xF0, 0x0F, 0x00, 0x00	
	    , 0x40, 0x01, 0x05, 0x80	
	    , 0x00, 0x01, 0x00, 0x00	
	    , 0x00, 0x01, 0x00, 0x00	
};

static UINT8 WMT_SET_DBGUART_REG_EVT[] = { 0x02, 0x08, 0x04, 0x00	
	    , 0x00		
	    , 0x00		
	    , 0x00		
	    , 0x02		
};
#endif

#if CFG_SET_OPT_REG_MCUIRQ	
static UINT8 WMT_SET_MCUIRQ_REG_CMD[] = { 0x01, 0x08, (4 + 12 * 4), 0x00	
	    , 0x01		
	    , 0x01		
	    , 0x00		
	    , 0x04		
	    , 0x00, 0x04, 0x00, 0x80	
	    , 0x03, 0x14, 0x00, 0x00	
	    , 0xFF, 0xFF, 0x00, 0x00	
	    
	    , 0x10, 0x01, 0x05, 0x80	
	    , 0x02, 0x00, 0x00, 0x00	
	    , 0x07, 0x00, 0x00, 0x00	
	    , 0x00, 0x01, 0x05, 0x80	
	    , 0x00, 0x00, 0x02, 0x00	
	    , 0x00, 0x00, 0x07, 0x00	
	    
	    , 0x80, 0x01, 0x05, 0x80	
	    , 0x1F, 0x1E, 0x00, 0x00	
	    , 0x7F, 0x7F, 0x00, 0x00	
};

static UINT8 WMT_SET_MCUIRQ_REG_EVT[] = { 0x02, 0x08, 0x04, 0x00	
	    , 0x00		
	    , 0x00		
	    , 0x00		
	    , 0x04		
};
#endif

static UINT8 WMT_SET_CRYSTAL_TRIMING_CMD[] = { 0x01, 0x12, 0x02, 0x00, 0x01, 0x00 };
static UINT8 WMT_SET_CRYSTAL_TRIMING_EVT[] = { 0x02, 0x12, 0x02, 0x00, 0x01, 0x00 };

static UINT8 WMT_GET_CRYSTAL_TRIMING_CMD[] = { 0x01, 0x12, 0x02, 0x00, 0x00, 0x00 };
static UINT8 WMT_GET_CRYSTAL_TRIMING_EVT[] = { 0x02, 0x12, 0x02, 0x00, 0x00, 0x00 };



#if CFG_WMT_FILTER_MODE_SETTING
static UINT8 WMT_COEX_EXT_COMPONENT_CMD[] = {0x01,0x10,0x03,0x00,0x0d,0x00,0x00};

static UINT8 WMT_COEX_FILTER_SPEC_CMD_TEST[] = {0x01,0x10,0x45,0x00,0x11,
					0x00,0x00,0x01,0x00,0x11,
					0x11,0x16,0x00,0x00,0x00,
					0x00,0x00,0x63,0x63,0x63,
					0x00,0x39,0x43,0x63,0x63,
					0x02,0x02,0x03,0x00,0x01,
					0x01,0x01,0x01,0x0e,0x0e,
					0x0e,0x00,0x0a,0x0c,0x0e,
					0x0e,0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0x00,0x00,
					0x00,0x00,0x00};

static UINT8 WMT_COEX_LTE_FREQ_IDX_TABLE_CMD[] = {0x01,0x10,0x21,0x00,0x12,
					0xfc,0x08,0x15,0x09,0x2e,
					0x09,0x47,0x09,0xc4,0x09,
					0xd4,0x09,0xe3,0x09,0x5a,
					0x0a,0x14,0x09,0x2d,0x09,
					0x46,0x09,0x60,0x09,0xd3,
					0x09,0xe2,0x09,0x59,0x0a,
					0x8B,0x0a};

static UINT8 WMT_COEX_LTE_CHAN_UNSAFE_CMD[] = {0x01,0x10,0x02,0x00,0x13,0x00};

#if CFG_WMT_LTE_ENABLE_MSGID_MAPPING
#else
static UINT8 WMT_COEX_IS_LTE_L_CMD[] = {0x01,0x10,0x02,0x00,0x21,0x01};
#endif

static UINT8 WMT_COEX_IS_LTE_PROJ_CMD[] = {0x01,0x10,0x02,0x00,0x15,0x01};

static UINT8 WMT_COEX_SPLIT_MODE_EVT[] = {0x02,0x10,0x01,0x00,0x00};
#endif

static struct init_script init_table_1_2[] = {
	INIT_CMD(WMT_QUERY_BAUD_CMD, WMT_QUERY_BAUD_EVT_115200, "query baud 115200"),
	INIT_CMD(WMT_QUERY_STP_CMD, WMT_QUERY_STP_EVT_DEFAULT, "query stp default"),
	INIT_CMD(WMT_SET_BAUD_CMD_X, WMT_SET_BAUD_EVT, "set baud rate"),
};


static struct init_script init_table_2[] = {
	INIT_CMD(WMT_QUERY_BAUD_CMD, WMT_QUERY_BAUD_EVT_X, "query baud X"),
};

static struct init_script init_table_3[] = {
	INIT_CMD(WMT_RESET_CMD, WMT_RESET_EVT, "wmt reset"),
#if CFG_WMT_BT_PORT2
	INIT_CMD(WMT_BTP2_CMD, WMT_BTP2_EVT, "set bt port2"),
#endif
};

static struct init_script set_crystal_timing_script[] = {
	INIT_CMD(WMT_SET_CRYSTAL_TRIMING_CMD, WMT_SET_CRYSTAL_TRIMING_EVT,
		 "set crystal trim value"),
};

static struct init_script get_crystal_timing_script[] = {
	INIT_CMD(WMT_GET_CRYSTAL_TRIMING_CMD, WMT_GET_CRYSTAL_TRIMING_EVT,
		 "get crystal trim value"),
};


static struct init_script init_table_4[] = {
	INIT_CMD(WMT_SET_STP_CMD, WMT_SET_STP_EVT, "set stp"),
};

static struct init_script init_table_5[] = {
	INIT_CMD(WMT_QUERY_STP_CMD, WMT_QUERY_STP_EVT_UART, "query stp uart"),
	INIT_CMD(WMT_QUERY_BAUD_CMD, WMT_QUERY_BAUD_EVT_X, "query baud X"),
};

static struct init_script init_table_5_1[] = {
	INIT_CMD(WMT_STRAP_CONF_CMD_FM_COMM, WMT_STRAP_CONF_EVT, "configure FM comm"),
};

static struct init_script init_table_6[] = {
	INIT_CMD(WMT_CORE_DUMP_LEVEL_04_CMD, WMT_CORE_DUMP_LEVEL_04_EVT, "setup core dump level"),
};


#if defined(CFG_SET_OPT_REG) && CFG_SET_OPT_REG
static struct init_script set_registers[] = {
	
	
#if CFG_WMT_I2S_DBGUART_SUPPORT
	INIT_CMD(WMT_SET_DBGUART_REG_CMD, WMT_SET_DBGUART_REG_EVT, "set debug uart registers"),
#endif
#if CFG_SET_OPT_REG_SWLA
	INIT_CMD(WMT_SET_SWLA_REG_CMD, WMT_SET_SWLA_REG_EVT, "set swla registers"),
#endif
#if CFG_SET_OPT_REG_MCUCLK
	INIT_CMD(WMT_SET_MCUCLK_REG_CMD, WMT_SET_MCUCLK_REG_EVT, "set mcuclk dbg registers"),
#endif
#if CFG_SET_OPT_REG_MCUIRQ
	INIT_CMD(WMT_SET_MCUIRQ_REG_CMD, WMT_SET_MCUIRQ_REG_EVT, "set mcu irq dbg registers"),
#endif
};
#endif

static struct init_script coex_table[] = {
	INIT_CMD(WMT_COEX_SETTING_CONFIG_CMD, WMT_COEX_SETTING_CONFIG_EVT, "coex_wmt"),

#if CFG_SUBSYS_COEX_NEED
	INIT_CMD(WMT_BT_COEX_SETTING_CONFIG_CMD, WMT_BT_COEX_SETTING_CONFIG_EVT, "coex_bt"),
	INIT_CMD(WMT_WIFI_COEX_SETTING_CONFIG_CMD, WMT_WIFI_COEX_SETTING_CONFIG_EVT, "coex_wifi"),
	INIT_CMD(WMT_PTA_COEX_SETTING_CONFIG_CMD, WMT_PTA_COEX_SETTING_CONFIG_EVT, "coex_ext_pta"),
	INIT_CMD(WMT_MISC_COEX_SETTING_CONFIG_CMD, WMT_MISC_COEX_SETTING_CONFIG_EVT, "coex_misc"),
#endif
};

static struct init_script osc_type_table[] = {
	INIT_CMD(WMT_CORE_CO_CLOCK_CMD, WMT_CORE_CO_CLOCK_EVT, "osc_type"),
};

#if (MTK_WCN_CMB_MERGE_INTERFACE_SUPPORT)
static struct init_script merge_pcm_table[] = {
	INIT_CMD(WMT_SET_DAI_MODE_REG_CMD, WMT_SET_DAI_MODE_REG_EVT, "DAI_PAD"),
};
#endif

#if CFG_WMT_FILTER_MODE_SETTING
#if CFG_WMT_LTE_ENABLE_MSGID_MAPPING
static struct init_script set_wifi_lte_coex_table_0[] = 
{
	INIT_CMD(WMT_COEX_EXT_COMPONENT_CMD, WMT_COEX_SPLIT_MODE_EVT, "wifi lte ext component"),
	INIT_CMD(WMT_COEX_FILTER_SPEC_CMD_TEST, WMT_COEX_SPLIT_MODE_EVT, "wifi lte coex filter"),
	INIT_CMD(WMT_COEX_LTE_FREQ_IDX_TABLE_CMD, WMT_COEX_SPLIT_MODE_EVT, "wifi lte freq id table"),
	INIT_CMD(WMT_COEX_LTE_CHAN_UNSAFE_CMD, WMT_COEX_SPLIT_MODE_EVT, "wifi lte unsafe channel"),
	INIT_CMD(WMT_COEX_IS_LTE_PROJ_CMD, WMT_COEX_SPLIT_MODE_EVT, "wifi coex is lte project"),
};
#else
static struct init_script set_wifi_lte_coex_table_0[] = 
{
	INIT_CMD(WMT_COEX_EXT_COMPONENT_CMD, WMT_COEX_SPLIT_MODE_EVT, "wifi lte ext component"),
	INIT_CMD(WMT_COEX_FILTER_SPEC_CMD_TEST, WMT_COEX_SPLIT_MODE_EVT, "wifi lte coex filter"),
	INIT_CMD(WMT_COEX_LTE_FREQ_IDX_TABLE_CMD, WMT_COEX_SPLIT_MODE_EVT, "wifi lte freq id table"),
	INIT_CMD(WMT_COEX_LTE_CHAN_UNSAFE_CMD, WMT_COEX_SPLIT_MODE_EVT, "wifi lte unsafe channel"),
	INIT_CMD(WMT_COEX_IS_LTE_L_CMD, WMT_COEX_SPLIT_MODE_EVT, "wifi coex is L branch"),
	INIT_CMD(WMT_COEX_IS_LTE_PROJ_CMD, WMT_COEX_SPLIT_MODE_EVT, "wifi coex is lte project"),
};
#endif
#endif

static const WMT_IC_INFO_S mt6630_info_table[] = {
    {
        .u4HwVer            = 0x8A00,
        .cChipName         = WMT_IC_NAME_MT6630,
        .cChipVersion      = WMT_IC_VER_E1,
        .cPatchNameExt     = WMT_IC_PATCH_E1_EXT,
        
        .eWmtHwVer        = WMTHWVER_E1,
        .bWorkWithoutPatch = MTK_WCN_BOOL_FALSE,
        .bPsmSupport       = MTK_WCN_BOOL_TRUE,
    },
    {
        .u4HwVer            = 0x8A10,
        .cChipName         = WMT_IC_NAME_MT6630,
        .cChipVersion      = WMT_IC_VER_E2,
        .cPatchNameExt     = WMT_IC_PATCH_E2_EXT,
        .eWmtHwVer        = WMTHWVER_E2,
        .bWorkWithoutPatch = MTK_WCN_BOOL_FALSE,
        .bPsmSupport       = MTK_WCN_BOOL_TRUE,
    },
    {
        .u4HwVer            = 0x8A11,
        .cChipName         = WMT_IC_NAME_MT6630,
        .cChipVersion      = WMT_IC_VER_E3,
        .cPatchNameExt     = WMT_IC_PATCH_E2_EXT,
        .eWmtHwVer        = WMTHWVER_E3,
        .bWorkWithoutPatch = MTK_WCN_BOOL_FALSE,
        .bPsmSupport       = MTK_WCN_BOOL_TRUE,
    },
    {
        .u4HwVer            = 0x8B11,
        .cChipName         = WMT_IC_NAME_MT6630,
        .cChipVersion      = WMT_IC_VER_E4,
        .cPatchNameExt     = WMT_IC_PATCH_E2_EXT,
        .eWmtHwVer        = WMTHWVER_E4,
        .bWorkWithoutPatch = MTK_WCN_BOOL_FALSE,
        .bPsmSupport       = MTK_WCN_BOOL_TRUE,
    }
};


static INT32
mt6630_sw_init(
    P_WMT_HIF_CONF pWmtHifConf
);

static INT32
mt6630_sw_deinit(
    P_WMT_HIF_CONF pWmtHifConf
);

static INT32
mt6630_pin_ctrl(
    WMT_IC_PIN_ID id,
    WMT_IC_PIN_STATE state,
    UINT32 flag
);

static INT32
mt6630_aif_ctrl(
    WMT_IC_PIN_STATE state,
    UINT32 flag
);

static INT32
mt6630_ver_check(VOID);

static const WMT_IC_INFO_S *
mt6630_find_wmt_ic_info(
    const UINT32 hw_ver
);

static INT32
wmt_stp_init_coex(VOID);

#if CFG_WMT_MULTI_PATCH
static INT32 mt6630_patch_dwn(UINT32 index);
static INT32 mt6630_patch_info_prepare(VOID);
#else
static INT32 mt6630_patch_dwn(VOID);
#endif

static INT32
mt6630_co_clock_ctrl(WMT_CO_CLOCK on);
static WMT_CO_CLOCK mt6630_co_clock_get(VOID);

static INT32
mt6630_crystal_triming_set(VOID);


static MTK_WCN_BOOL mt6630_quick_sleep_flag_get(VOID);

static MTK_WCN_BOOL mt6630_aee_dump_flag_get(VOID);
#if 0
static UINT8 WMT_SET_SDIO_DRV_REG_CMD[] = { 0x01, 0x08, 0x10, 0x00	
	    , 0x01		
	    , 0x01		
	    , 0x00		
	    , 0x01		
	    , 0x50, 0x00, 0x05, 0x80	
	    , 0x44, 0x44, 0x04, 0x00	
	    , 0x77, 0x77, 0x07, 0x00	
};

static UINT8 WMT_SET_SDIO_DRV_REG_EVT[] = { 0x02, 0x08, 0x04, 0x00	
	    , 0x00		
	    , 0x00		
	    , 0x00		
	    , 0x01		
};

static INT32 mt6630_set_sdio_driving(void);
static struct init_script sdio_driving_table[] = {
	INIT_CMD(WMT_SET_SDIO_DRV_REG_CMD, WMT_SET_SDIO_DRV_REG_EVT, "sdio_driving"),
};

#endif
static MTK_WCN_BOOL mt6630_trigger_stp_assert(VOID);
#if CFG_WMT_FILTER_MODE_SETTING
static INT32 wmt_stp_wifi_lte_coex (VOID);
#endif



const WMT_IC_OPS wmt_ic_ops_mt6630 = {
	.icId = 0x6630,
	.sw_init = mt6630_sw_init,
	.sw_deinit = mt6630_sw_deinit,
	.ic_pin_ctrl = mt6630_pin_ctrl,
	.ic_ver_check = mt6630_ver_check,
	.co_clock_ctrl = mt6630_co_clock_ctrl,
	.is_quick_sleep = mt6630_quick_sleep_flag_get,
	.is_aee_dump_support = mt6630_aee_dump_flag_get,
	.trigger_stp_assert = mt6630_trigger_stp_assert,
};


static INT32 mt6630_sw_init(P_WMT_HIF_CONF pWmtHifConf)
{
	INT32 iRet = -1;
	UINT32 u4Res = 0;
	UINT8 evtBuf[256];
	unsigned long ctrlPa1;
	unsigned long ctrlPa2;
	UINT32 hw_ver;
#if CFG_WMT_MULTI_PATCH
	UINT32 patch_num = 0;
	UINT32 patch_index = 0;
#endif
	WMT_CTRL_DATA ctrlData;
	WMT_DBG_FUNC(" start\n");

	osal_assert(NULL != gp_mt6630_info);

	if ((NULL == gp_mt6630_info)
	    || (NULL == pWmtHifConf)
	    ) {
		WMT_ERR_FUNC("null pointers: gp_mt6630_info(0x%p), pWmtHifConf(0x%p)\n",
			     gp_mt6630_info, pWmtHifConf);
		return -1;
	}

	hw_ver = gp_mt6630_info->u4HwVer;

	

	
	if (WMT_HIF_UART == pWmtHifConf->hifType) {
		
		osal_memcpy(&WMT_SET_BAUD_CMD_X[5], &pWmtHifConf->au4HifConf[0],
			    osal_sizeof(UINT32));
		WMT_SET_BAUD_CMD_X[8] = (UINT8) 0x00;	  
		osal_memcpy(&WMT_QUERY_BAUD_EVT_X[6], &pWmtHifConf->au4HifConf[0],
			    osal_sizeof(UINT32));
		WMT_QUERY_BAUD_EVT_X[9] = (UINT8) 0x00;	 

		
		
		
		
		iRet = wmt_core_init_script(init_table_1_2, ARRAY_SIZE(init_table_1_2));

		if (iRet) {
			WMT_ERR_FUNC("init_table_1_2 fail(%d)\n", iRet);
			osal_assert(0);
			return -2;
		}

		
		ctrlPa1 = pWmtHifConf->au4HifConf[0];
		ctrlPa2 = 0;
		iRet = wmt_core_ctrl(WMT_CTRL_HOST_BAUDRATE_SET, &ctrlPa1, &ctrlPa2);

		if (iRet) {
			WMT_ERR_FUNC("change baudrate(%d) fail(%d)\n", pWmtHifConf->au4HifConf[0],
				     iRet);
			return -3;
		}

		WMT_INFO_FUNC("WMT-CORE: change baudrate(%d) ok\n", pWmtHifConf->au4HifConf[0]);

		
		iRet =
		    wmt_core_tx((PUINT8) & WMT_SET_WAKEUP_WAKE_CMD_RAW[0], 1, &u4Res,
				MTK_WCN_BOOL_TRUE);

		if (iRet || (u4Res != 1)) {
			WMT_ERR_FUNC("write raw iRet(%d) written(%d)\n", iRet, u4Res);
			return -4;
		}

		osal_memset(evtBuf, 0, osal_sizeof(evtBuf));
		iRet = wmt_core_rx(evtBuf, osal_sizeof(WMT_SET_WAKEUP_WAKE_EVT), &u4Res);
#ifdef CFG_DUMP_EVT
		WMT_DBG_FUNC("WAKEUP_WAKE_EVT read len %d [%02x,%02x,%02x,%02x,%02x,%02x]\n",
			     (INT32) u4Res, evtBuf[0], evtBuf[1], evtBuf[2], evtBuf[3], evtBuf[4],
			     evtBuf[5]);
#endif

		if (iRet || (u4Res != osal_sizeof(WMT_SET_WAKEUP_WAKE_EVT))) {
			WMT_ERR_FUNC("read WAKEUP_WAKE_EVT fail(%d)\n", iRet);
			return -5;
		}
		

#if CFG_CHECK_WMT_RESULT

		if (osal_memcmp
		    (evtBuf, WMT_SET_WAKEUP_WAKE_EVT, osal_sizeof(WMT_SET_WAKEUP_WAKE_EVT)) != 0) {
			WMT_ERR_FUNC("WMT-CORE: write WMT_SET_WAKEUP_WAKE_CMD_RAW status fail\n");
			return -6;
		}
#endif

		
		iRet = wmt_core_init_script(init_table_2, osal_array_size(init_table_2));

		if (iRet) {
			WMT_ERR_FUNC("init_table_2 fail(%d)\n", iRet);
			return -7;
		}
	}

	
#if CFG_WMT_MULTI_PATCH
	
	iRet = mt6630_patch_info_prepare();

	if (iRet) {
		WMT_ERR_FUNC("patch info perpare fail(%d)\n", iRet);
		return -8;
	}

	
	ctrlPa1 = 0;
	ctrlPa2 = 0;
	wmt_core_ctrl(WMT_CTRL_GET_PATCH_NUM, &ctrlPa1, &ctrlPa2);
	patch_num = ctrlPa1;
	WMT_INFO_FUNC("patch total num = [%d]\n", patch_num);

	
	for (patch_index = 0; patch_index < patch_num; patch_index++) {
		iRet = mt6630_patch_dwn(patch_index);

		if (iRet) {
			WMT_ERR_FUNC("patch dwn fail (%d),patch_index(%d)\n", iRet, patch_index);
			return -12;
		}

		iRet = wmt_core_init_script(init_table_3, ARRAY_SIZE(init_table_3));

		if (iRet) {
			WMT_ERR_FUNC("init_table_3 fail(%d)\n", iRet);
			return -13;
		}
	}

#else
	
	iRet = mt6630_patch_dwn();

	
	if (iRet) {
		WMT_ERR_FUNC("patch dwn fail (%d), just omit\n", iRet);
	}
#endif				

	
	iRet = wmt_core_init_script(init_table_3, ARRAY_SIZE(init_table_3));

	if (iRet) {
		WMT_ERR_FUNC("init_table_3 fail(%d)\n", iRet);
		return -9;
	}

	iRet = wmt_stp_init_coex();

	if (iRet) {
		WMT_ERR_FUNC("init_coex fail(%d)\n", iRet);
		return -10;
	} else {
		WMT_INFO_FUNC("init_coex ok\n");
	}

	mt6630_crystal_triming_set();
#if MT6630_BRINGUP
	WMT_INFO_FUNC("Bring up period, skip sdio driving settings\n");
#else
	WMT_INFO_FUNC("Temp solution, skip sdio driving settings\n");
	
#endif
	if (WMT_HIF_UART == pWmtHifConf->hifType) {
		
		iRet = wmt_core_init_script(init_table_4, ARRAY_SIZE(init_table_4));

		if (iRet) {
			WMT_ERR_FUNC("init_table_4 fail(%d)\n", iRet);
			return -12;
		}

		
		ctrlPa1 = WMT_STP_CONF_MODE;
		ctrlPa2 = MTKSTP_UART_FULL_MODE;
		iRet = wmt_core_ctrl(WMT_CTRL_STP_CONF, &ctrlPa1, &ctrlPa2);
		ctrlPa1 = WMT_STP_CONF_EN;
		ctrlPa2 = 1;
		iRet += wmt_core_ctrl(WMT_CTRL_STP_CONF, &ctrlPa1, &ctrlPa2);

		if (iRet) {
			WMT_ERR_FUNC("enable host STP-UART-FULL mode fail(%d)\n", iRet);
			return -13;
		}

		WMT_INFO_FUNC("enable host STP-UART-FULL mode\n");
		
		osal_sleep_ms(10);
		
		
		iRet = wmt_core_init_script(init_table_5, ARRAY_SIZE(init_table_5));

		if (iRet) {
			WMT_ERR_FUNC("init_table_5 fail(%d)\n", iRet);
			return -14;
		}
	}

	if (WMT_CO_CLOCK_EN == mt6630_co_clock_get()) {
		WMT_INFO_FUNC("co-clock enabled.\n");

		iRet = wmt_core_init_script(osc_type_table, ARRAY_SIZE(osc_type_table));

		if (iRet) {
			WMT_ERR_FUNC("osc_type_table fail(%d), goes on\n", iRet);
			return -15;
		}
	} else {
		WMT_INFO_FUNC("co-clock disabled.\n");
	}
#if MT6630_BRINGUP
	WMT_INFO_FUNC("Bring up period, skip merge interface settings\n");
#else

#if (MTK_WCN_CMB_MERGE_INTERFACE_SUPPORT)
	iRet = wmt_core_init_script(merge_pcm_table, ARRAY_SIZE(merge_pcm_table));

	if (iRet) {
		WMT_ERR_FUNC("merge_pcm_table fail(%d), goes on\n", iRet);
		return -15;
	}
#endif
#endif
	
	WMT_STRAP_CONF_CMD_FM_COMM[5] = (UINT8) pWmtHifConf->au4StrapConf[0];
	WMT_STRAP_CONF_EVT[5] = (UINT8) pWmtHifConf->au4StrapConf[0];
	iRet = wmt_core_init_script(init_table_5_1, ARRAY_SIZE(init_table_5_1));

	if (iRet) {
		WMT_ERR_FUNC("init_table_5_1 fm mode(%d) fail(%d)\n",
			     pWmtHifConf->au4StrapConf[0], iRet);
		return -16;
	}

	WMT_INFO_FUNC("set fm mode (%d) ok\n", pWmtHifConf->au4StrapConf[0]);

#if CFG_SET_OPT_REG		
	iRet = wmt_core_init_script(set_registers, ARRAY_SIZE(set_registers));

	if (iRet) {
		WMT_ERR_FUNC("set_registers fail(%d)", iRet);
		return -17;
	}
#endif

#if CFG_WMT_COREDUMP_ENABLE
	
	mtk_wcn_stp_coredump_flag_ctrl(1);
#endif

	if (0 != mtk_wcn_stp_coredump_flag_get()) {
		iRet = wmt_core_init_script(init_table_6, ARRAY_SIZE(init_table_6));

		if (iRet) {
			WMT_ERR_FUNC("init_table_6 core dump setting fail(%d)\n", iRet);
			return -18;
		} else {
			WMT_INFO_FUNC("enable mt662x firmware coredump\n");
		}
	} else {
		WMT_INFO_FUNC("disable mt662x firmware coredump\n");
	}

	ctrlData.ctrlId = WMT_CTRL_SET_STP_DBG_INFO;
	ctrlData.au4CtrlData[0] = wmt_ic_ops_mt6630.icId;
	ctrlData.au4CtrlData[1] = (size_t) gp_mt6630_info->cChipVersion;
	ctrlData.au4CtrlData[2] = (size_t) &gp_mt6630_patch_info;
	iRet = wmt_ctrl(&ctrlData);
	if (iRet) {
		WMT_ERR_FUNC("set dump info fail(%d)\n", iRet);
		return -16;
	}
	
#if CFG_WMT_FILTER_MODE_SETTING
	wmt_stp_wifi_lte_coex();
	WMT_DBG_FUNC("wmt_stp_wifi_lte_coex done!\n");
#endif		

#if CFG_WMT_PS_SUPPORT
	osal_assert(NULL != gp_mt6630_info);

	if (NULL != gp_mt6630_info) {
		if (MTK_WCN_BOOL_FALSE != gp_mt6630_info->bPsmSupport) {
			wmt_lib_ps_enable();
		} else {
			wmt_lib_ps_disable();
		}
	}
#endif

	return 0;
}

static INT32 mt6630_sw_deinit(P_WMT_HIF_CONF pWmtHifConf)
{
	WMT_DBG_FUNC(" start\n");

#if CFG_WMT_PS_SUPPORT
	osal_assert(NULL != gp_mt6630_info);

	if ((NULL != gp_mt6630_info)
	    && (MTK_WCN_BOOL_FALSE != gp_mt6630_info->bPsmSupport)) {
		wmt_lib_ps_disable();
	}
#endif

	gp_mt6630_info = NULL;

	return 0;
}

static INT32 mt6630_aif_ctrl(WMT_IC_PIN_STATE state, UINT32 flag)
{
	INT32 ret = -1;

#if MT6630_BRINGUP
	ret = 0;
	WMT_INFO_FUNC("Bring up period, skip aif settings\n");
#else

	if ((flag & WMT_LIB_AIF_FLAG_MASK) == WMT_LIB_AIF_FLAG_SHARE) {
		WMT_INFO_FUNC("PCM & I2S PIN SHARE\n");

		WMT_WARN_FUNC("TBD!!");
		ret = 0;
	} else {
		
		WMT_INFO_FUNC("PCM & I2S PIN SEPARATE\n");

		switch (state) {
		case WMT_IC_AIF_0:
			
			ret = 0;
			break;

		case WMT_IC_AIF_1:
			
			ret = 0;
			break;

		case WMT_IC_AIF_2:
			
#if 0
			val = 0x01110000;
			ret = wmt_core_reg_rw_raw(1, 0x80050078, &val, 0x0FFF0000);
#else
			ret = 0;
			WMT_INFO_FUNC("Bring up period, skip WMT_IC_AIF_2 settings\n");
#endif
			break;

		case WMT_IC_AIF_3:
			ret = 0;
			break;

		default:
			WMT_ERR_FUNC("unsupported state (%d)\n", state);
			ret = -1;
			break;
		}
	}

	if (!ret) {
		WMT_INFO_FUNC("new state(%d) ok\n", state);
	} else {
		WMT_WARN_FUNC("new state(%d) fail(%d)\n", state, ret);
	}
#endif
	return ret;
}

static INT32 mt6630_gps_sync_ctrl(WMT_IC_PIN_STATE state, UINT32 flag)
{
	INT32 iRet = -1;
	UINT32 uVal = 0;

	if (WMT_IC_PIN_MUX == state) {
		uVal = 0x1 << 1;
	} else {
		uVal = 0x0 << 1;
	}
	
#if MT6630_BRINGUP
	iRet = 0;
	WMT_INFO_FUNC("Bring up period, skip gps sync settings\n");

#else
	iRet = wmt_core_reg_rw_raw(1, 0x80025070, &uVal, 0xf << 4);
#endif
	if (0 != iRet) {
		WMT_ERR_FUNC("gps_sync pin ctrl failed, iRet(%d)\n", iRet);
	}
	
	return 0;
}


static INT32 mt6630_pin_ctrl(WMT_IC_PIN_ID id, WMT_IC_PIN_STATE state, UINT32 flag)
{
	INT32 ret;

	WMT_DBG_FUNC("ic pin id:%d, state:%d, flag:0x%x\n", id, state, flag);

	ret = -1;

	switch (id) {
	case WMT_IC_PIN_AUDIO:
		ret = mt6630_aif_ctrl(state, flag);
		break;

	case WMT_IC_PIN_EEDI:
		WMT_WARN_FUNC("TBD!!");
		
		ret = 0;
		break;

	case WMT_IC_PIN_EEDO:
		WMT_WARN_FUNC("TBD!!");
		
		ret = 0;
		break;

	case WMT_IC_PIN_GSYNC:
		ret = mt6630_gps_sync_ctrl(state, flag);
		break;

	default:
		break;
	}

	WMT_INFO_FUNC("ret = (%d)\n", ret);

	return ret;
}

INT32 mt6630_co_clock_ctrl(WMT_CO_CLOCK on)
{
	INT32 iRet = 0;

	if ((WMT_CO_CLOCK_DIS <= on) && (WMT_CO_CLOCK_MAX > on)) {
		gCoClockEn = on;
	} else {
		WMT_DBG_FUNC("MT6630: error parameter:%d\n", on);
		iRet = -1;
	}

	WMT_DBG_FUNC("MT6630: Co-clock %s\n",
		     (gCoClockEn == WMT_CO_CLOCK_DIS) ? "disabled" : "enabled");

	return iRet;
}

static MTK_WCN_BOOL mt6630_quick_sleep_flag_get(VOID)
{
	return MTK_WCN_BOOL_TRUE;
}


static MTK_WCN_BOOL mt6630_aee_dump_flag_get(VOID)
{
	return MTK_WCN_BOOL_TRUE;
}

static MTK_WCN_BOOL mt6630_trigger_stp_assert(VOID)
{
	INT32 iRet = -1;
	UINT32 u4Res = 0;
	MTK_WCN_BOOL bRet = MTK_WCN_BOOL_FALSE;
	UINT8 STP_DO_ASSERT_CMD[] =
	    { 0x80, 0x50, 0x0a, 0x00, 'd', 'o', 'c', 'o', 'r', 'e', 'd', 'u', 'm', 'p', 0x00,
0x00 };

	iRet =
	    wmt_core_tx((PUINT8) & STP_DO_ASSERT_CMD[0], sizeof(STP_DO_ASSERT_CMD), &u4Res,
			MTK_WCN_BOOL_TRUE);
	if (iRet || (u4Res != sizeof(STP_DO_ASSERT_CMD))) {
		WMT_ERR_FUNC("wmt_core:send STP ASSERT COMMAND fail(%d),size(%d)\n", iRet, u4Res);
		bRet = MTK_WCN_BOOL_FALSE;
	} else
		bRet = MTK_WCN_BOOL_TRUE;
	return bRet;
}

WMT_CO_CLOCK mt6630_co_clock_get(VOID)
{
	return gCoClockEn;
}



static INT32 mt6630_ver_check(VOID)
{
	UINT32 hw_ver;
	UINT32 fw_ver;
	INT32 iret;
	const WMT_IC_INFO_S *p_info;
	unsigned long ctrlPa1;
	unsigned long ctrlPa2;

	
	WMT_LOUD_FUNC("MT6630: before read hw_ver (hw version)\n");
	iret = wmt_core_reg_rw_raw(0, GEN_HVR, &hw_ver, GEN_VER_MASK);

	if (iret) {
		WMT_ERR_FUNC("MT6630: read hw_ver fail:%d\n", iret);
		return -2;
	}

	WMT_INFO_FUNC("MT6630: read hw_ver (hw version) (0x%x)\n", hw_ver);

	WMT_LOUD_FUNC("MT6630: before fw_ver (rom version)\n");
	wmt_core_reg_rw_raw(0, GEN_FVR, &fw_ver, GEN_VER_MASK);

	if (iret) {
		WMT_ERR_FUNC("MT6630: read fw_ver fail:%d\n", iret);
		return -2;
	}

	WMT_INFO_FUNC("MT6630: read fw_ver (rom version) (0x%x)\n", fw_ver);

	p_info = mt6630_find_wmt_ic_info(hw_ver);

	if (NULL == p_info) {
		WMT_ERR_FUNC("MT6630: hw_ver(0x%x) find wmt ic info fail\n", hw_ver);
		return -3;
	}

	WMT_INFO_FUNC("MT6630: wmt ic info: %s.%s (0x%x, WMTHWVER:%d, patch_ext:%s)\n",
		      p_info->cChipName, p_info->cChipVersion,
		      p_info->u4HwVer, p_info->eWmtHwVer, p_info->cPatchNameExt);

	
	ctrlPa1 = (0x00006630UL << 16) | (hw_ver & 0x0000FFFF);
	
	ctrlPa2 = ((UINT32) (p_info->eWmtHwVer) << 16) | (fw_ver & 0x0000FFFF);

	iret = wmt_core_ctrl(WMT_CTRL_HWIDVER_SET, &ctrlPa1, &ctrlPa2);

	if (iret) {
		WMT_WARN_FUNC("MT6630: WMT_CTRL_HWIDVER_SET fail(%d)\n", iret);
	}

	gp_mt6630_info = p_info;
	return 0;
}

static const WMT_IC_INFO_S *mt6630_find_wmt_ic_info(const UINT32 hw_ver)
{
	
	const UINT32 size = ARRAY_SIZE(mt6630_info_table);
	INT32 index;

	
	
	
	index = size - 1;

	
	while ((0 <= index)
	       && (hw_ver != mt6630_info_table[index].u4HwVer)	
	    ) {
		--index;
	}

	if (0 <= index) {
		WMT_INFO_FUNC("found ic info(0x%x) by full match! index:%d\n", hw_ver, index);
		return &mt6630_info_table[index];
	}

	WMT_WARN_FUNC("find no ic info for (0x%x) by full match!try major num match!\n", hw_ver);

	
	index = size - 1;

	
	while ((0 <= index)
	       && (MAJORNUM(hw_ver) != MAJORNUM(mt6630_info_table[index].u4HwVer))
	    ) {
		--index;
	}

	if (0 <= index) {
		WMT_INFO_FUNC("MT6630: found ic info for hw_ver(0x%x) by major num! index:%d\n",
			      hw_ver, index);
		return &mt6630_info_table[index];
	}

	WMT_ERR_FUNC
	    ("MT6630: find no ic info for hw_ver(0x%x) by full match nor major num match!\n",
	     hw_ver);
	return NULL;
}


static INT32 wmt_stp_init_coex(VOID)
{
	INT32 iRet;
	unsigned long addr;
	WMT_GEN_CONF *pWmtGenConf;

#define COEX_WMT  0

#if CFG_SUBSYS_COEX_NEED
	
#define COEX_BT   1
#define COEX_WIFI 2
#define COEX_PTA  3
#define COEX_MISC 4
#endif
	
	iRet = wmt_core_ctrl(WMT_CTRL_GET_WMT_CONF, &addr, 0);

	if (iRet) {
		WMT_ERR_FUNC("ctrl GET_WMT_CONF fail(%d)\n", iRet);
		return -2;
	}

	WMT_INFO_FUNC("ctrl GET_WMT_CONF ok(0x%x)\n", addr);

	pWmtGenConf = (P_WMT_GEN_CONF) addr;

	
	if (pWmtGenConf->cfgExist == 0) {
		WMT_INFO_FUNC("cfgExist == 0, skip config chip\n");
		
		return 0;
	}


	
	WMT_DBG_FUNC("coex_wmt:0x%x\n", pWmtGenConf->coex_wmt_ant_mode);
#if CFG_SUBSYS_COEX_NEED
	WMT_DBG_FUNC("coex_bt:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		     pWmtGenConf->coex_bt_rssi_upper_limit,
		     pWmtGenConf->coex_bt_rssi_mid_limit,
		     pWmtGenConf->coex_bt_rssi_lower_limit,
		     pWmtGenConf->coex_bt_pwr_high,
		     pWmtGenConf->coex_bt_pwr_mid, pWmtGenConf->coex_bt_pwr_low);
	WMT_DBG_FUNC("coex_wifi:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		     pWmtGenConf->coex_wifi_rssi_upper_limit,
		     pWmtGenConf->coex_wifi_rssi_mid_limit,
		     pWmtGenConf->coex_wifi_rssi_lower_limit,
		     pWmtGenConf->coex_wifi_pwr_high,
		     pWmtGenConf->coex_wifi_pwr_mid, pWmtGenConf->coex_wifi_pwr_low);
	WMT_DBG_FUNC("coex_ext_pta:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		     pWmtGenConf->coex_ext_pta_hi_tx_tag,
		     pWmtGenConf->coex_ext_pta_hi_rx_tag,
		     pWmtGenConf->coex_ext_pta_lo_tx_tag,
		     pWmtGenConf->coex_ext_pta_lo_rx_tag,
		     pWmtGenConf->coex_ext_pta_sample_t1,
		     pWmtGenConf->coex_ext_pta_sample_t2,
		     pWmtGenConf->coex_ext_pta_wifi_bt_con_trx);
	WMT_DBG_FUNC("coex_misc:0x%x 0x%x 0x%x\n",
		     pWmtGenConf->coex_misc_ext_pta_on, pWmtGenConf->coex_misc_ext_feature_set);
#endif

	
	coex_table[COEX_WMT].cmd[5] = pWmtGenConf->coex_wmt_ant_mode;

	if (gWmtDbgLvl >= WMT_LOG_DBG) {
		wmt_core_dump_data(&coex_table[COEX_WMT].cmd[0],
				   coex_table[COEX_WMT].str, coex_table[COEX_WMT].cmdSz);
	}
#if CFG_SUBSYS_COEX_NEED
	coex_table[COEX_BT].cmd[9] = pWmtGenConf->coex_bt_rssi_upper_limit;
	coex_table[COEX_BT].cmd[10] = pWmtGenConf->coex_bt_rssi_mid_limit;
	coex_table[COEX_BT].cmd[11] = pWmtGenConf->coex_bt_rssi_lower_limit;
	coex_table[COEX_BT].cmd[12] = pWmtGenConf->coex_bt_pwr_high;
	coex_table[COEX_BT].cmd[13] = pWmtGenConf->coex_bt_pwr_mid;
	coex_table[COEX_BT].cmd[14] = pWmtGenConf->coex_bt_pwr_low;

	if (gWmtDbgLvl >= WMT_LOG_DBG) {
		wmt_core_dump_data(&coex_table[COEX_BT].cmd[0],
				   coex_table[COEX_BT].str, coex_table[COEX_BT].cmdSz);
	}

	coex_table[COEX_WIFI].cmd[10] = pWmtGenConf->coex_wifi_rssi_upper_limit;
	coex_table[COEX_WIFI].cmd[11] = pWmtGenConf->coex_wifi_rssi_mid_limit;
	coex_table[COEX_WIFI].cmd[12] = pWmtGenConf->coex_wifi_rssi_lower_limit;
	coex_table[COEX_WIFI].cmd[13] = pWmtGenConf->coex_wifi_pwr_high;
	coex_table[COEX_WIFI].cmd[14] = pWmtGenConf->coex_wifi_pwr_mid;
	coex_table[COEX_WIFI].cmd[15] = pWmtGenConf->coex_wifi_pwr_low;

	if (gWmtDbgLvl >= WMT_LOG_DBG) {
		wmt_core_dump_data(&coex_table[COEX_WIFI].cmd[0],
				   coex_table[COEX_WIFI].str, coex_table[COEX_WIFI].cmdSz);
	}

	coex_table[COEX_PTA].cmd[5] = pWmtGenConf->coex_ext_pta_hi_tx_tag;
	coex_table[COEX_PTA].cmd[6] = pWmtGenConf->coex_ext_pta_hi_rx_tag;
	coex_table[COEX_PTA].cmd[7] = pWmtGenConf->coex_ext_pta_lo_tx_tag;
	coex_table[COEX_PTA].cmd[8] = pWmtGenConf->coex_ext_pta_lo_rx_tag;
	coex_table[COEX_PTA].cmd[9] = ((pWmtGenConf->coex_ext_pta_sample_t1 & 0xff00) >> 8);
	coex_table[COEX_PTA].cmd[10] = ((pWmtGenConf->coex_ext_pta_sample_t1 & 0x00ff) >> 0);
	coex_table[COEX_PTA].cmd[11] = ((pWmtGenConf->coex_ext_pta_sample_t2 & 0xff00) >> 8);
	coex_table[COEX_PTA].cmd[12] = ((pWmtGenConf->coex_ext_pta_sample_t2 & 0x00ff) >> 0);
	coex_table[COEX_PTA].cmd[13] = pWmtGenConf->coex_ext_pta_wifi_bt_con_trx;

	if (gWmtDbgLvl >= WMT_LOG_DBG) {
		wmt_core_dump_data(&coex_table[COEX_PTA].cmd[0],
				   coex_table[COEX_PTA].str, coex_table[COEX_PTA].cmdSz);
	}

	osal_memcpy(&coex_table[COEX_MISC].cmd[5], &pWmtGenConf->coex_misc_ext_pta_on,
		    sizeof(pWmtGenConf->coex_misc_ext_pta_on));
	osal_memcpy(&coex_table[COEX_MISC].cmd[9], &pWmtGenConf->coex_misc_ext_feature_set,
		    sizeof(pWmtGenConf->coex_misc_ext_feature_set));

	wmt_core_dump_data(&coex_table[COEX_MISC].cmd[0], coex_table[COEX_MISC].str,
			   coex_table[COEX_MISC].cmdSz);
#endif

	iRet = wmt_core_init_script(coex_table, ARRAY_SIZE(coex_table));

	return iRet;
}

#if 0
static INT32 mt6630_set_sdio_driving(void)
{
	INT32 ret = 0;

	unsigned long addr;
	WMT_GEN_CONF *pWmtGenConf;
	UINT32 drv_val = 0;

	
	ret = wmt_core_ctrl(WMT_CTRL_GET_WMT_CONF, &addr, 0);

	if (ret) {
		WMT_ERR_FUNC("ctrl GET_WMT_CONF fail(%d)\n", ret);
		return -1;
	}

	WMT_INFO_FUNC("ctrl GET_WMT_CONF ok(0x%x)\n", addr);

	pWmtGenConf = (P_WMT_GEN_CONF) addr;

	
	if (pWmtGenConf->cfgExist == 0) {
		WMT_INFO_FUNC("cfgExist == 0, skip config chip\n");
		
		return 0;
	}

	drv_val = pWmtGenConf->sdio_driving_cfg;

	
	WMT_INFO_FUNC("sdio driving:0x%x\n", drv_val);

	sdio_driving_table[0].cmd[12] = (UINT8) ((drv_val & 0x00000077UL) >> 0);	
	sdio_driving_table[0].cmd[13] = (UINT8) ((drv_val & 0x00007700UL) >> 8);	
	sdio_driving_table[0].cmd[14] = (UINT8) ((drv_val & 0x00070000UL) >> 16);	

	ret =
	    wmt_core_init_script(sdio_driving_table,
				 ARRAY_SIZE(sdio_driving_table));

	return ret;
}
#endif

static INT32 mt6630_crystal_triming_set(VOID)
{
	INT32 iRet = 0;
	PUINT8 pbuf = NULL;
	UINT32 bufLen = 0;
	WMT_CTRL_DATA ctrlData;
	UINT32 uCryTimOffset = 0x6D;
	MTK_WCN_BOOL bIsNvramExist = MTK_WCN_BOOL_FALSE;
	INT8 cCrystalTimingOffset = 0x0;
	UINT8 cCrystalTiming = 0x0;
	INT32 iCrystalTiming = 0x0;
	MTK_WCN_BOOL bIsCrysTrimEnabled = MTK_WCN_BOOL_FALSE;
	UINT32 u4Res;
	bIsNvramExist = MTK_WCN_BOOL_FALSE;
	  ctrlData.ctrlId = WMT_CTRL_CRYSTAL_TRIMING_GET;
	ctrlData.au4CtrlData[0] = (size_t) "/data/nvram/APCFG/APRDEB/WIFI";
	ctrlData.au4CtrlData[1] = (size_t) &pbuf;
	ctrlData.au4CtrlData[2] = (size_t) &bufLen;

	iRet = wmt_ctrl(&ctrlData);

	if (0 != iRet) {
		WMT_ERR_FUNC("MT6630: WMT_CTRL_CRYSTAL_TRIMING_GET fail:%d\n", iRet);
		bIsNvramExist = MTK_WCN_BOOL_FALSE;
		bIsCrysTrimEnabled = MTK_WCN_BOOL_FALSE;
		cCrystalTimingOffset = 0x0;
		cCrystalTiming = 0x0;
		iRet = -1;
	} else {
		WMT_DBG_FUNC("MT6630: nvram pBuf(%p), bufLen(%d)\n", pbuf, bufLen);

		if (bufLen < (uCryTimOffset + 1)) {
			WMT_ERR_FUNC
			    ("MT6630: nvram len(%d) too short, crystalTimging value offset(%d)\n",
			     bufLen, uCryTimOffset);
			bIsNvramExist = MTK_WCN_BOOL_FALSE;
			bIsCrysTrimEnabled = MTK_WCN_BOOL_FALSE;
			cCrystalTimingOffset = 0x0;
			cCrystalTiming = 0x0;
		} else {
			bIsNvramExist = MTK_WCN_BOOL_TRUE;
			cCrystalTimingOffset = *(pbuf + uCryTimOffset);

			if (cCrystalTimingOffset & 0x80) {
				bIsCrysTrimEnabled = MTK_WCN_BOOL_TRUE;
				cCrystalTimingOffset = (UINT8) cCrystalTimingOffset & 0x7f;
			}

			WMT_DBG_FUNC("cCrystalTimingOffset (%d), bIsCrysTrimEnabled(%d)\n",
				     cCrystalTimingOffset, bIsCrysTrimEnabled);
		}

		ctrlData.ctrlId = WMT_CTRL_CRYSTAL_TRIMING_PUT;
		ctrlData.au4CtrlData[0] = (size_t) "/data/nvram/APCFG/APRDEB/WIFI";
		iRet = wmt_ctrl(&ctrlData);

		if (0 != iRet) {
			WMT_ERR_FUNC("MT6630: WMT_CTRL_CRYSTAL_TRIMING_PUT fail:%d\n", iRet);
			iRet = -2;
		} else {
			WMT_DBG_FUNC("MT6630: WMT_CTRL_CRYSTAL_TRIMING_PUT succeed\n");
		}
	}

	if ((MTK_WCN_BOOL_TRUE == bIsNvramExist) && (MTK_WCN_BOOL_TRUE == bIsCrysTrimEnabled)) {
		
		iRet =
		    wmt_core_tx(get_crystal_timing_script[0].cmd,
				get_crystal_timing_script[0].cmdSz, &u4Res, MTK_WCN_BOOL_FALSE);

		if (iRet || (u4Res != get_crystal_timing_script[0].cmdSz)) {
			WMT_ERR_FUNC("WMT-CORE: write (%s) iRet(%d) cmd len err(%d, %d)\n",
				     get_crystal_timing_script[0].str, iRet, u4Res,
				     get_crystal_timing_script[0].cmdSz);
			iRet = -3;
			goto done;
		}

		
		osal_memset(get_crystal_timing_script[0].evt, 0,
			    get_crystal_timing_script[0].evtSz);
		iRet =
		    wmt_core_rx(get_crystal_timing_script[0].evt,
				get_crystal_timing_script[0].evtSz, &u4Res);

		if (iRet || (u4Res != get_crystal_timing_script[0].evtSz)) {
			WMT_ERR_FUNC("WMT-CORE: read (%s) iRet(%d) evt len err(rx:%d, exp:%d)\n",
				     get_crystal_timing_script[0].str, iRet, u4Res,
				     get_crystal_timing_script[0].evtSz);
			mtk_wcn_stp_dbg_dump_package();
			iRet = -4;
			goto done;
		}

		iCrystalTiming = WMT_GET_CRYSTAL_TRIMING_EVT[5] & 0x7f;

		if (cCrystalTimingOffset & 0x40) {
			
			iCrystalTiming = iCrystalTiming + cCrystalTimingOffset - 128;
		} else {
			iCrystalTiming += cCrystalTimingOffset;
		}

		WMT_DBG_FUNC("iCrystalTiming (0x%x)\n", iCrystalTiming);
		cCrystalTiming = iCrystalTiming > 0x7f ? 0x7f : iCrystalTiming;
		cCrystalTiming = iCrystalTiming < 0 ? 0 : iCrystalTiming;
		WMT_DBG_FUNC("cCrystalTiming (0x%x)\n", cCrystalTiming);
		
		WMT_SET_CRYSTAL_TRIMING_CMD[5] = cCrystalTiming;
		WMT_GET_CRYSTAL_TRIMING_EVT[5] = cCrystalTiming;

		iRet =
		    wmt_core_init_script(set_crystal_timing_script,
					 ARRAY_SIZE(set_crystal_timing_script));

		if (iRet) {
			WMT_ERR_FUNC("set_crystal_timing_script fail(%d)\n", iRet);
			iRet = -5;
		} else {
			WMT_DBG_FUNC("set crystal timing value (0x%x) succeed\n",
				     WMT_SET_CRYSTAL_TRIMING_CMD[5]);
			iRet =
			    wmt_core_init_script(get_crystal_timing_script,
						 ARRAY_SIZE(get_crystal_timing_script));

			if (iRet) {
				WMT_ERR_FUNC("get_crystal_timing_script fail(%d)\n", iRet);
				iRet = -6;
			} else {
				WMT_INFO_FUNC("succeed, updated crystal timing value (0x%x)\n",
					      WMT_GET_CRYSTAL_TRIMING_EVT[5]);
				iRet = 0x0;
			}
		}
	}

 done:
	return iRet;
}


#if CFG_WMT_MULTI_PATCH
static INT32 mt6630_patch_info_prepare(VOID)
{
	INT32 iRet = -1;
	WMT_CTRL_DATA ctrlData;

	ctrlData.ctrlId = WMT_CTRL_PATCH_SEARCH;
	iRet = wmt_ctrl(&ctrlData);

	return iRet;
}


static INT32 mt6630_patch_dwn(UINT32 index)
{
	INT32 iRet = -1;
	P_WMT_PATCH patchHdr;
	PUINT8 pbuf;
	UINT32 patchSize;
	UINT32 fragSeq;
	UINT32 fragNum;
	UINT16 fragSize = 0;
	UINT16 cmdLen;
	UINT32 offset;
	UINT32 u4Res;
	UINT8 evtBuf[8];
	UINT8 addressevtBuf[12];
	UINT8 addressByte[4];
	PINT8 cDataTime = NULL;
	
	UINT16 u2HwVer = 0;
	UINT16 u2SwVer = 0;
	UINT32 u4PatchVer = 0;
	UINT32 patchSizePerFrag = 0;
	WMT_CTRL_DATA ctrlData;

	
	if (NULL == gp_mt6630_info) {
		WMT_ERR_FUNC("null gp_mt6630_info!\n");
		return -1;
	}

	osal_memset(gFullPatchName, 0, osal_sizeof(gFullPatchName));

	ctrlData.ctrlId = WMT_CTRL_GET_PATCH_INFO;
	ctrlData.au4CtrlData[0] = index + 1;
	ctrlData.au4CtrlData[1] = (size_t) &gFullPatchName;
	ctrlData.au4CtrlData[2] = (size_t) &addressByte;
	iRet = wmt_ctrl(&ctrlData);
	WMT_INFO_FUNC("the %d time valid patch found: (%s)\n", index + 1, gFullPatchName);

	
	ctrlData.ctrlId = WMT_CTRL_GET_PATCH;
	ctrlData.au4CtrlData[0] = (size_t) NULL;
	ctrlData.au4CtrlData[1] = (size_t) &gFullPatchName;
	ctrlData.au4CtrlData[2] = (size_t) &pbuf;
	ctrlData.au4CtrlData[3] = (size_t) &patchSize;
	iRet = wmt_ctrl(&ctrlData);

	if (iRet) {
		WMT_ERR_FUNC("wmt_core: WMT_CTRL_GET_PATCH fail:%d\n", iRet);
		iRet -= 1;
		goto done;
	}

	
	pbuf += BCNT_PATCH_BUF_HEADROOM;
	patchHdr = (P_WMT_PATCH) pbuf;
	

	cDataTime = patchHdr->ucDateTime;
	u2HwVer = patchHdr->u2HwVer;
	u2SwVer = patchHdr->u2SwVer;
	u4PatchVer = patchHdr->u4PatchVer;

	osal_memcpy(&gp_mt6630_patch_info, patchHdr, osal_sizeof(WMT_PATCH));

	

	cDataTime[15] = '\0';

	if (index == 0) {
		WMT_INFO_FUNC("===========================================\n");
		WMT_INFO_FUNC("[Combo Patch] Built Time = %s\n", cDataTime);
		WMT_INFO_FUNC("[Combo Patch] Hw Ver = 0x%x\n",
			      ((u2HwVer & 0x00ff) << 8) | ((u2HwVer & 0xff00) >> 8));
		WMT_INFO_FUNC("[Combo Patch] Sw Ver = 0x%x\n",
			      ((u2SwVer & 0x00ff) << 8) | ((u2SwVer & 0xff00) >> 8));
		WMT_INFO_FUNC("[Combo Patch] Ph Ver = 0x%04x\n",
			      ((u4PatchVer & 0xff000000) >> 24) | ((u4PatchVer & 0x00ff0000) >>
								   16));
		WMT_INFO_FUNC("[Combo Patch] Platform = %c%c%c%c\n", patchHdr->ucPLat[0],
			      patchHdr->ucPLat[1], patchHdr->ucPLat[2], patchHdr->ucPLat[3]);
		WMT_INFO_FUNC("===========================================\n");
	}

	patchSize -= sizeof(WMT_PATCH);
	pbuf += sizeof(WMT_PATCH);
	patchSizePerFrag = DEFAULT_PATCH_FRAG_SIZE;
	pbuf -= sizeof(WMT_PATCH_CMD);

	fragNum = patchSize / patchSizePerFrag;
	fragNum += ((fragNum * patchSizePerFrag) == patchSize) ? 0 : 1;

	WMT_DBG_FUNC("patch size(%d) fragNum(%d)\n", patchSize, fragNum);


	
	iRet =
	    wmt_core_tx((PUINT8) &WMT_PATCH_ADDRESS_CMD[0], sizeof(WMT_PATCH_ADDRESS_CMD), &u4Res,
			MTK_WCN_BOOL_FALSE);

	if (iRet || (u4Res != sizeof(WMT_PATCH_ADDRESS_CMD))) {
		WMT_ERR_FUNC("wmt_core:wmt patch address CMD fail(%d),size(%d)\n", iRet, u4Res);
		iRet -= 1;
		goto done;
	}

	osal_memset(addressevtBuf, 0, sizeof(addressevtBuf));
	iRet = wmt_core_rx(addressevtBuf, sizeof(WMT_PATCH_ADDRESS_EVT), &u4Res);

	if (iRet || (u4Res != sizeof(WMT_PATCH_ADDRESS_EVT))) {
		WMT_ERR_FUNC("wmt_core:wmt patch address EVT fail(%d),size(%d)\n", iRet, u4Res);
		iRet -= 1;
		goto done;
	}
#if CFG_CHECK_WMT_RESULT

	if (osal_memcmp(addressevtBuf, WMT_PATCH_ADDRESS_EVT, osal_sizeof(WMT_PATCH_ADDRESS_EVT)) !=
	    0) {
		WMT_ERR_FUNC("wmt_core: write WMT_PATCH_ADDRESS_CMD status fail\n");
		iRet -= 1;
		goto done;
	}
#endif

	
	osal_memcpy(&WMT_PATCH_P_ADDRESS_CMD[12], addressByte, osal_sizeof(addressByte));
	WMT_INFO_FUNC("4 bytes address command:0x%02x,0x%02x,0x%02x,0x%02x",
		      WMT_PATCH_P_ADDRESS_CMD[12],
		      WMT_PATCH_P_ADDRESS_CMD[13],
		      WMT_PATCH_P_ADDRESS_CMD[14], WMT_PATCH_P_ADDRESS_CMD[15]);
	iRet =
	    wmt_core_tx((PUINT8) &WMT_PATCH_P_ADDRESS_CMD[0], sizeof(WMT_PATCH_P_ADDRESS_CMD),
			&u4Res, MTK_WCN_BOOL_FALSE);

	if (iRet || (u4Res != sizeof(WMT_PATCH_P_ADDRESS_CMD))) {
		WMT_ERR_FUNC("wmt_core:wmt part patch address CMD fail(%d),size(%d),index(%d)\n",
			     iRet, u4Res, index);
		iRet -= 1;
		goto done;
	}

	osal_memset(addressevtBuf, 0, sizeof(addressevtBuf));
	iRet = wmt_core_rx(addressevtBuf, sizeof(WMT_PATCH_P_ADDRESS_EVT), &u4Res);

	if (iRet || (u4Res != sizeof(WMT_PATCH_P_ADDRESS_EVT))) {
		WMT_ERR_FUNC("wmt_core:wmt patch address EVT fail(%d),size(%d),index(%d)\n", iRet,
			     u4Res, index);
		iRet -= 1;
		goto done;
	}
#if CFG_CHECK_WMT_RESULT

	if (osal_memcmp(addressevtBuf, WMT_PATCH_P_ADDRESS_EVT, osal_sizeof(WMT_PATCH_ADDRESS_EVT))
	    != 0) {
		WMT_ERR_FUNC("wmt_core: write WMT_PATCH_ADDRESS_CMD status fail,index(%d)\n",
			     index);
		iRet -= 1;
		goto done;
	}
#endif

	
	offset = sizeof(WMT_PATCH_CMD);
	fragSeq = 0;

	while (fragSeq < fragNum) {
		WMT_DBG_FUNC("patch size(%d) fragNum(%d)\n", patchSize, fragNum);

		if (fragSeq == (fragNum - 1)) {
			
			fragSize = patchSize - fragSeq * patchSizePerFrag;
			WMT_PATCH_CMD[4] = WMT_PATCH_FRAG_LAST;
		} else {
			fragSize = patchSizePerFrag;
			WMT_PATCH_CMD[4] = (fragSeq == 0) ? WMT_PATCH_FRAG_1ST : WMT_PATCH_FRAG_MID;
		}

		
		cmdLen = 1 + fragSize;
		osal_memcpy(&WMT_PATCH_CMD[2], &cmdLen, 2);
		
		osal_memcpy(pbuf + offset - sizeof(WMT_PATCH_CMD), WMT_PATCH_CMD,
			    sizeof(WMT_PATCH_CMD));

		
		iRet =
		    wmt_core_tx(pbuf + offset - sizeof(WMT_PATCH_CMD),
				fragSize + sizeof(WMT_PATCH_CMD), &u4Res, MTK_WCN_BOOL_FALSE);

		if (iRet || (u4Res != fragSize + sizeof(WMT_PATCH_CMD))) {
			WMT_ERR_FUNC("wmt_core: write fragSeq(%d) size(%d, %d) fail(%d)\n", fragSeq,
				     fragSize + sizeof(WMT_PATCH_CMD), u4Res, iRet);
			iRet -= 1;
			break;
		}

		WMT_DBG_FUNC("wmt_core: write fragSeq(%d) size(%d, %d) ok\n",
			     fragSeq, fragSize + sizeof(WMT_PATCH_CMD), u4Res);

		osal_memset(evtBuf, 0, sizeof(evtBuf));
		
		iRet = wmt_core_rx(evtBuf, sizeof(WMT_PATCH_EVT), &u4Res);

		if (iRet || (u4Res != sizeof(WMT_PATCH_EVT))) {
			WMT_ERR_FUNC("wmt_core: read WMT_PATCH_EVT length(%d, %d) fail(%d)\n",
				     sizeof(WMT_PATCH_EVT), u4Res, iRet);
			iRet -= 1;
			break;
		}
#if CFG_CHECK_WMT_RESULT

		if (osal_memcmp(evtBuf, WMT_PATCH_EVT, sizeof(WMT_PATCH_EVT)) != 0) {
			WMT_ERR_FUNC
			    ("wmt_core: compare WMT_PATCH_EVT result error rx(%d):[%02X,%02X,%02X,%02X,%02X] exp(%d):[%02X,%02X,%02X,%02X,%02X]\n",
			     u4Res, evtBuf[0], evtBuf[1], evtBuf[2], evtBuf[3], evtBuf[4],
			     sizeof(WMT_PATCH_EVT), WMT_PATCH_EVT[0], WMT_PATCH_EVT[1],
			     WMT_PATCH_EVT[2], WMT_PATCH_EVT[3], WMT_PATCH_EVT[4]);
			iRet -= 1;
			break;
		}
#endif
		WMT_DBG_FUNC("wmt_core: read WMT_PATCH_EVT length(%d, %d) ok\n",
			     sizeof(WMT_PATCH_EVT), u4Res);
		offset += patchSizePerFrag;
		++fragSeq;
	}

	WMT_INFO_FUNC("wmt_core: patch dwn:%d frag(%d, %d) %s\n",
		      iRet, fragSeq, fragSize, (!iRet && (fragSeq == fragNum)) ? "ok" : "fail");

	if (fragSeq != fragNum) {
		iRet -= 1;
	}

 done:
	
	
	ctrlData.ctrlId = WMT_CTRL_FREE_PATCH;
	ctrlData.au4CtrlData[0] = index + 1;
	wmt_ctrl(&ctrlData);

	return iRet;
}

#else
static INT32 mt6630_patch_dwn(VOID)
{
	INT32 iRet = -1;
	P_WMT_PATCH patchHdr;
	PUINT8 pbuf;
	UINT32 patchSize;
	UINT32 fragSeq;
	UINT32 fragNum;
	UINT16 fragSize = 0;
	UINT16 cmdLen;
	UINT32 offset;
	UINT32 u4Res;
	UINT8 evtBuf[8];
	PINT8 cDataTime = NULL;
	
	UINT16 u2HwVer = 0;
	UINT16 u2SwVer = 0;
	UINT32 u4PatchVer = 0;
	UINT32 patchSizePerFrag = 0;
	WMT_CTRL_DATA ctrlData;

	
	if (NULL == gp_mt6630_info) {
		WMT_ERR_FUNC("null gp_mt6630_info!\n");
		return -1;
	}
	
	
	ctrlData.ctrlId = WMT_CTRL_PATCH_SEARCH;
	iRet = wmt_ctrl(&ctrlData);

	if (0 == iRet) {
		
		ctrlData.ctrlId = WMT_CTRL_GET_PATCH_NAME;
		ctrlData.au4CtrlData[0] = (size_t) &gFullPatchName;
		iRet = wmt_ctrl(&ctrlData);

		WMT_INFO_FUNC("valid patch found: (%s)\n", gFullPatchName);
		
		ctrlData.ctrlId = WMT_CTRL_GET_PATCH;
		ctrlData.au4CtrlData[0] = (size_t) NULL;
		ctrlData.au4CtrlData[1] = (size_t) &gFullPatchName;

	} else {
		iRet -= 1;
		return iRet;
	}

	ctrlData.au4CtrlData[2] = (size_t) &pbuf;
	ctrlData.au4CtrlData[3] = (size_t) &patchSize;
	iRet = wmt_ctrl(&ctrlData);

	if (iRet) {
		WMT_ERR_FUNC("wmt_core: WMT_CTRL_GET_PATCH fail:%d\n", iRet);
		iRet -= 1;
		goto done;
	}

	
	pbuf += BCNT_PATCH_BUF_HEADROOM;
	patchHdr = (P_WMT_PATCH) pbuf;
	

	cDataTime = patchHdr->ucDateTime;
	u2HwVer = patchHdr->u2HwVer;
	u2SwVer = patchHdr->u2SwVer;
	u4PatchVer = patchHdr->u4PatchVer;

	osal_memcpy(&gp_mt6630_patch_info, patchHdr, osal_sizeof(WMT_PATCH));
	

	cDataTime[15] = '\0';
	WMT_INFO_FUNC("===========================================\n");
	WMT_INFO_FUNC("[Combo Patch] Built Time = %s\n", cDataTime);
	WMT_INFO_FUNC("[Combo Patch] Hw Ver = 0x%x\n",
		      ((u2HwVer & 0x00ff) << 8) | ((u2HwVer & 0xff00) >> 8));
	WMT_INFO_FUNC("[Combo Patch] Sw Ver = 0x%x\n",
		      ((u2SwVer & 0x00ff) << 8) | ((u2SwVer & 0xff00) >> 8));
	WMT_INFO_FUNC("[Combo Patch] Ph Ver = 0x%04x\n",
		      ((u4PatchVer & 0xff000000) >> 24) | ((u4PatchVer & 0x00ff0000) >> 16));
	WMT_INFO_FUNC("[Combo Patch] Platform = %c%c%c%c\n", patchHdr->ucPLat[0],
		      patchHdr->ucPLat[1], patchHdr->ucPLat[2], patchHdr->ucPLat[3]);
	WMT_INFO_FUNC("===========================================\n");

	patchSize -= sizeof(WMT_PATCH);
	pbuf += sizeof(WMT_PATCH);
	patchSizePerFrag = DEFAULT_PATCH_FRAG_SIZE;
	pbuf -= sizeof(WMT_PATCH_CMD);

	fragNum = patchSize / patchSizePerFrag;
	fragNum += ((fragNum * patchSizePerFrag) == patchSize) ? 0 : 1;

	WMT_DBG_FUNC("patch size(%d) fragNum(%d)\n", patchSize, fragNum);


	
	offset = sizeof(WMT_PATCH_CMD);
	fragSeq = 0;

	while (fragSeq < fragNum) {
		WMT_DBG_FUNC("patch size(%d) fragNum(%d)\n", patchSize, fragNum);

		if (fragSeq == (fragNum - 1)) {
			
			fragSize = patchSize - fragSeq * patchSizePerFrag;
			WMT_PATCH_CMD[4] = WMT_PATCH_FRAG_LAST;
		} else {
			fragSize = patchSizePerFrag;
			WMT_PATCH_CMD[4] = (fragSeq == 0) ? WMT_PATCH_FRAG_1ST : WMT_PATCH_FRAG_MID;
		}

		
		cmdLen = 1 + fragSize;
		osal_memcpy(&WMT_PATCH_CMD[2], &cmdLen, 2);
		
		osal_memcpy(pbuf + offset - sizeof(WMT_PATCH_CMD), WMT_PATCH_CMD,
			    sizeof(WMT_PATCH_CMD));

		
		iRet =
		    wmt_core_tx(pbuf + offset - sizeof(WMT_PATCH_CMD),
				fragSize + sizeof(WMT_PATCH_CMD), &u4Res, MTK_WCN_BOOL_FALSE);

		if (iRet || (u4Res != fragSize + sizeof(WMT_PATCH_CMD))) {
			WMT_ERR_FUNC("wmt_core: write fragSeq(%d) size(%d, %d) fail(%d)\n", fragSeq,
				     fragSize + sizeof(WMT_PATCH_CMD), u4Res, iRet);
			iRet -= 1;
			break;
		}

		WMT_DBG_FUNC("wmt_core: write fragSeq(%d) size(%d, %d) ok\n",
			     fragSeq, fragSize + sizeof(WMT_PATCH_CMD), u4Res);

		osal_memset(evtBuf, 0, sizeof(evtBuf));
		
		iRet = wmt_core_rx(evtBuf, sizeof(WMT_PATCH_EVT), &u4Res);

		if (iRet || (u4Res != sizeof(WMT_PATCH_EVT))) {
			WMT_ERR_FUNC("wmt_core: read WMT_PATCH_EVT length(%d, %d) fail(%d)\n",
				     sizeof(WMT_PATCH_EVT), u4Res, iRet);
			iRet -= 1;
			break;
		}
#if CFG_CHECK_WMT_RESULT

		if (osal_memcmp(evtBuf, WMT_PATCH_EVT, sizeof(WMT_PATCH_EVT)) != 0) {
			WMT_ERR_FUNC
			    ("wmt_core: compare WMT_PATCH_EVT result error rx(%d):[%02X,%02X,%02X,%02X,%02X] exp(%d):[%02X,%02X,%02X,%02X,%02X]\n",
			     u4Res, evtBuf[0], evtBuf[1], evtBuf[2], evtBuf[3], evtBuf[4],
			     sizeof(WMT_PATCH_EVT), WMT_PATCH_EVT[0], WMT_PATCH_EVT[1],
			     WMT_PATCH_EVT[2], WMT_PATCH_EVT[3], WMT_PATCH_EVT[4]);
			iRet -= 1;
			break;
		}
#endif
		WMT_DBG_FUNC("wmt_core: read WMT_PATCH_EVT length(%d, %d) ok\n",
			     sizeof(WMT_PATCH_EVT), u4Res);
		offset += patchSizePerFrag;
		++fragSeq;
	}

	WMT_INFO_FUNC("wmt_core: patch dwn:%d frag(%d, %d) %s\n",
		      iRet, fragSeq, fragSize, (!iRet && (fragSeq == fragNum)) ? "ok" : "fail");

	if (fragSeq != fragNum) {
		iRet -= 1;
	}

 done:
	
	wmt_core_ctrl(WMT_CTRL_FREE_PATCH, NULL, NULL);

	return iRet;
}

#endif

#if CFG_WMT_FILTER_MODE_SETTING
static INT32
wmt_stp_wifi_lte_coex (VOID)
{
    INT32 iRet;
    unsigned long addr;
    WMT_GEN_CONF *pWmtGenConf;

    
    iRet = wmt_core_ctrl(WMT_CTRL_GET_WMT_CONF, &addr, 0);
    if (iRet) {
        WMT_ERR_FUNC("ctrl GET_WMT_CONF fail(%d)\n", iRet);
        return -2;
    }
    WMT_INFO_FUNC("ctrl GET_WMT_CONF ok(0x%08lx)\n", addr);

    pWmtGenConf = (P_WMT_GEN_CONF)addr;

    
    if (pWmtGenConf->cfgExist == 0) {
        WMT_INFO_FUNC("cfgExist == 0, skip config chip\n");
        
        return 0;
    }
	
	if(pWmtGenConf->coex_wmt_filter_mode == 0)
	{
		iRet = wmt_core_init_script(set_wifi_lte_coex_table_0, ARRAY_SIZE(set_wifi_lte_coex_table_0));
		if(iRet)
		{
			WMT_ERR_FUNC("wmt_core:set_wifi_lte_coex_table_0 fail(%d)\n",iRet);
		}else
		{
			WMT_INFO_FUNC("wmt_core:set_wifi_lte_coex_table_0 ok\n");
		}
	}

    return iRet;
}
#endif
