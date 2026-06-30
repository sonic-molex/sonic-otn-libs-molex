/*****************************************************************************************************
   Copyright (C) 2012, Oplink Communications Inc.
   FileName:      dev_data.h
   Author:        Zhen Zhang
   Date:          2018-06-01
   Version:       1.0
   Description:
   Function List:

   History:
   [Zhen Zhang] [2018-06-01] [1.0] [Creator]


*****************************************************************************************************/

#ifndef _DEV_DATA__
#define _DEV_DATA__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t uiState;

} CLedCfg, CLedSta;

#define LED_MAX_NUM                          32


#define MFG_INFO_LEN_MAX                     64

typedef struct
{
    char acManufacturer[MFG_INFO_LEN_MAX];
    char acProductName[MFG_INFO_LEN_MAX];
    char acModuleType[MFG_INFO_LEN_MAX];
    char acBootVer[MFG_INFO_LEN_MAX];
    char acSwVer[MFG_INFO_LEN_MAX];
    char acFwVer[MFG_INFO_LEN_MAX];
    char acFpgaRunVer[MFG_INFO_LEN_MAX];
    char acFpga1Ver[MFG_INFO_LEN_MAX];         /* Chassis */
    char acFpga2Ver[MFG_INFO_LEN_MAX];             /* Scc */
    char acCpldRunVer[MFG_INFO_LEN_MAX];       /*CPLD0, used for control powers*/
    char acCpld1Ver[MFG_INFO_LEN_MAX];         /* CPLD1 */
    char acCpld2Ver[MFG_INFO_LEN_MAX];         /* reserved*/
    char acHwVer[MFG_INFO_LEN_MAX];
    char acHwFuncRev[MFG_INFO_LEN_MAX];
    char acPcbRev[MFG_INFO_LEN_MAX];
    char acSN[MFG_INFO_LEN_MAX];
    char acPN[MFG_INFO_LEN_MAX];
    char acMfgDate[MFG_INFO_LEN_MAX];
    char acCaliDate[MFG_INFO_LEN_MAX];
    char acCleiCode[MFG_INFO_LEN_MAX];
    char acSubBdlVer[MFG_INFO_LEN_MAX];
    char acEepromVer[MFG_INFO_LEN_MAX];
    char acEth0Mac[MFG_INFO_LEN_MAX];
    char acEth1Mac[MFG_INFO_LEN_MAX];
    char acEth2Mac[MFG_INFO_LEN_MAX];
    char acEth3Mac[MFG_INFO_LEN_MAX];
    char acEth4Mac[MFG_INFO_LEN_MAX];
    char acEth5Mac[MFG_INFO_LEN_MAX];
    char acEth6Mac[MFG_INFO_LEN_MAX];
    char acEth7Mac[MFG_INFO_LEN_MAX];
    char acEth8Mac[MFG_INFO_LEN_MAX];
    char acEth9Mac[MFG_INFO_LEN_MAX];

    /* fnc additional */
    char acUnitName[MFG_INFO_LEN_MAX];
    char acFCPN[MFG_INFO_LEN_MAX];
    char acFCIss[MFG_INFO_LEN_MAX];
    char acFujitsuPN[MFG_INFO_LEN_MAX];

    char acConfigType[MFG_INFO_LEN_MAX];
    char acFjtSn[32];

    char acResv[32];
}CMfg;

#define COM_CALI_MAX_LEN       1024
#define COM_CALI_RSV_LEN       (COM_CALI_MAX_LEN - (4 * 8 * 2) - (80 * 2))

typedef struct
{
    /* Line  8 */
    int16_t sLineInToPaIn;
    int16_t sLineInToBaIn;
    int16_t sPaOutToLineOut;
    int16_t sBaOutToLineOut;
    int16_t asRsv1[4];

    /* OCM   8 */
    int16_t sPaInToOcm;
    int16_t sPaOutToOcm;
    int16_t sBaInToOcm;
    int16_t sBaOutToOcm;
    int16_t sWss1InToOcm;
    int16_t sWss1OutToOcm;
    int16_t sWss2InToOcm;
    int16_t sWss2OutToOcm;

    /* OSC    8 */
    int16_t sLineInToOsc1Drop;
    int16_t sOsc1AddToLineOut;
    int16_t sLineInToOsc1Rx;
    int16_t sOsc1TxToLineOut;
    int16_t sLineInToOsc2Drop;
    int16_t sOsc2AddToLineOut;
    int16_t sLineInToOsc2Rx;
    int16_t sOsc2TxToLineOut;

    /* Mirror 4 */
    int16_t sMirrorTxToMuxIn;
    int16_t sMirrorTxToDemuxIn;
    int16_t sMuxInToMirrorRx;
    int16_t sDemuxInToMirrorRx;

    /* Ocm External */
    int16_t sOcmPortToOcm;

    /* Reserve 3 */
    int16_t asRsv4[3];

    /* Express  80 */
    int16_t asExpInToBaIn[20];
    int16_t asRsv5[20];
    int16_t asPaOutToExpOut[20];
    int16_t asRsv6[20];

    int16_t asRsv7[COM_CALI_RSV_LEN];
} CCali;

#define FUNC_FLAG_OFFSET 16

#if 1
enum EFuncType
{
    FUNC_TYPE_NONE = 0,

    FUNC_TYPE_P_OTA,
    FUNC_TYPE_WSS,
    FUNC_TYPE_OPS,

    FUNC_TYPE_NUM
};


enum EDevType
{
    DEV_TYPE_NONE = 0,

    /* OA */
    DEV_TYPE_OA,

    DEV_TYPE_BA,
    DEV_TYPE_PA,
    DEV_TYPE_BA_PA,
    DEV_TYPE_ASE,
    DEV_TYPE_W_OA,     /* west oa in ILA or ILE*/
    DEV_TYPE_E_OA,     /* east oa in ILA or ILE */

    /* WSS */
    DEV_TYPE_WSS,
    DEV_TYPE_WSS1,
    DEV_TYPE_WSS2,
    DEV_TYPE_WSS3,

    /* OCM */
    DEV_TYPE_OCM,

    /* OPS */
    DEV_TYPE_OPS,
    DEV_TYPE_OPS4A,
    DEV_TYPE_OPS4B,

    /* OSC */
    DEV_TYPE_OSC,

    DEV_TYPE_W_OSC,
    DEV_TYPE_E_OSC,

    /* VOA */
    DEV_TYPE_VOA,

    DEV_TYPE_PADDING_VOA,
    DEV_TYPE_PA_VOA,
    DEV_TYPE_OSC_VOA,

    /* TDCM */
    DEV_TYPE_TDCM,

    /* PAM4 */
    DEV_TYPE_PAM4,

    /* DCE */
    DEV_TYPE_DCE,

    /* OTDR */
    DEV_TYPE_OTDR,

    /* MUX */
    DEV_TYPE_MUX,

    /* Optical Cross connect switch */
    DEV_TYPE_OCS,

    /* EXTEND */
    DEV_TYPE_EXTEND,

    /* LED */
    DEV_TYPE_LED,
    DEV_TYPE_SYS_LED,
    DEV_TYPE_ALM_LED,

    /* COLD_RESET */
    DEV_TYPE_COLD_RESET,

    DEV_TYPE_OA_COLD_RESET,
    DEV_TYPE_BA_COLD_RESET,
    DEV_TYPE_PA_COLD_RESET,
    DEV_TYPE_BA_PA_COLD_RESET,
    DEV_TYPE_ASE_COLD_RESET,

    DEV_TYPE_WSS_COLD_RESET,
    DEV_TYPE_WSS1_COLD_RESET,
    DEV_TYPE_WSS2_COLD_RESET,
    DEV_TYPE_WSS1_WARM_RESET,
    DEV_TYPE_WSS2_WARM_RESET,

    DEV_TYPE_OPS_COLD_RESET,

    DEV_TYPE_OCM_COLD_RESET,
    DEV_TYPE_WSS_OCM_COLD_RESET,

    DEV_TYPE_OSC_COLD_RESET,
    DEV_TYPE_W_OSC_COLD_RESET,
    DEV_TYPE_E_OSC_COLD_RESET,

    DEV_TYPE_OTDR_COLD_RESET,

    DEV_TYPE_OCS_COLD_RESET,


    DEV_TYPE_E2PROM,
    DEV_TYPE_TEMP,
    DEV_TYPE_PHY,

    DEV_TYPE_SCC,
    DEV_TYPE_FPGA,
    DEV_TYPE_CPLD,

    DEV_TYPE_P_BA = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_BA,         /* use in P-OTA */
    DEV_TYPE_P_PA = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_PA,     /* use in P-OTA */
    DEV_TYPE_P_BA_PA = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_BA_PA,    /* internal type for hal */

    DEV_TYPE_P_OCM = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_OCM,
    DEV_TYPE_WSS_OCM = (FUNC_TYPE_WSS << FUNC_FLAG_OFFSET) + DEV_TYPE_OCM,

    DEV_TYPE_P_OSC = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_OSC,
    DEV_TYPE_WSS_OSC = (FUNC_TYPE_WSS << FUNC_FLAG_OFFSET) + DEV_TYPE_OSC,

    DEV_TYPE_P_PADDING_VOA = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_PADDING_VOA,
    DEV_TYPE_P_PA_VOA = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_PA_VOA,
    DEV_TYPE_P_OSC_VOA = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_OSC_VOA,

    DEV_TYPE_P_OTA_SYS_LED = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_SYS_LED,
    DEV_TYPE_P_OTA_ALM_LED = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_ALM_LED,
    DEV_TYPE_WSS_SYS_LED = (FUNC_TYPE_WSS << FUNC_FLAG_OFFSET) + DEV_TYPE_SYS_LED,
    DEV_TYPE_WSS_ALM_LED = (FUNC_TYPE_WSS << FUNC_FLAG_OFFSET) + DEV_TYPE_ALM_LED,
    DEV_TYPE_OPS_SYS_LED = (FUNC_TYPE_OPS << FUNC_FLAG_OFFSET) + DEV_TYPE_SYS_LED,
    DEV_TYPE_OPS_ALM_LED = (FUNC_TYPE_OPS << FUNC_FLAG_OFFSET) + DEV_TYPE_ALM_LED,

    DEV_TYPE_P_BA_COLD_RESET = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_BA_COLD_RESET,
    DEV_TYPE_P_PA_COLD_RESET = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_PA_COLD_RESET,
    DEV_TYPE_P_BA_PA_COLD_RESET = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_BA_PA_COLD_RESET,
    DEV_TYPE_P_OCM_COLD_RESET = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_OCM_COLD_RESET,
    DEV_TYPE_P_OSC_COLD_RESET = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_OSC_COLD_RESET,
    DEV_TYPE_P_OTDR_COLD_RESET = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_OTDR_COLD_RESET,

    DEV_TYPE_P_OTDR = (FUNC_TYPE_P_OTA << FUNC_FLAG_OFFSET) + DEV_TYPE_OTDR,
};

enum ELogicType
{
    LOGIC_TYPE_CTRL = 0,
    LOGIC_TYPE_MAIN
};


enum EModuleType
{
    MODULE_TYPE_NONE,
    MODULE_TYPE_CHASSIS,
    MODULE_TYPE_SCC,
    MODULE_TYPE_SLOT_INFO,

    MODULE_TYPE_OA,
    MODULE_TYPE_VOA,
    MODULE_TYPE_WSS,
    MODULE_TYPE_OCM,
    MODULE_TYPE_TDCM,
    MODULE_TYPE_PAM4,
    MODULE_TYPE_OPS,
    MODULE_TYPE_DCE,
    MODULE_TYPE_MCS,
    MODULE_TYPE_OSC,
    MODULE_TYPE_DGE,
    MODULE_TYPE_OTDR,
    MODULE_TYPE_EXTEND,
    MODULE_TYPE_MUX,
    MODULE_TYPE_OCS,
    MODULE_TYPE_ADC,

    MODULE_TYPE_FPGA,
    MODULE_TYPE_CPLD,
    MODULE_TYPE_FAN,
    MODULE_TYPE_LED,
    MODULE_TYPE_TEMP,
    MODULE_TYPE_POWER,
    MODULE_TYPE_E2PROM,
    MODULE_TYPE_FLASH,
    MODULE_TYPE_RESET,
    MODULE_TYPE_UPGRADE,

    UNKONW_TYPE
};

enum EModuleFuncType
{
    MODULE_FUNC_TYPE_NONE,
    MODULE_FUNC_TYPE_OA,
    MODULE_FUNC_TYPE_OA_PA,
    MODULE_FUNC_TYPE_OA_BA,
    MODULE_FUNC_TYPE_OTDR,
    MODULE_FUNC_TYPE_OCM,
    MODULE_FUNC_TYPE_OPS,
    MODULE_FUNC_TYPE_EXTEND,
    MODULE_FUNC_TYPE_MUX,
    MODULE_FUNC_TYPE_TDCM,
    MODULE_FUNC_TYPE_OSC,
    MODULE_FUNC_TYPE_WSS0,
    MODULE_FUNC_TYPE_WSS1,
};


#else

enum EDevType
{
    DEV_TYPE_NONE,
    DEV_TYPE_CHASSIS,
    DEV_TYPE_SCC,
    DEV_TYPE_SLOT_INFO,

    DEV_TYPE_OA,
    DEV_TYPE_VOA,
    DEV_TYPE_WSS,
    DEV_TYPE_OCM,
    DEV_TYPE_TDCM,
    DEV_TYPE_PAM4,
    DEV_TYPE_OPS,
    DEV_TYPE_DCE,
    DEV_TYPE_MCS,
    DEV_TYPE_OSC,
    DEV_TYPE_DGE,
    DEV_TYPE_OTDR,
    DEV_TYPE_EXTEND,
    DEV_TYPE_MUX,
    DEV_TYPE_ADC,

    DEV_TYPE_FPGA,
    DEV_TYPE_CPLD,
    DEV_TYPE_FAN,
    DEV_TYPE_LED,
    DEV_TYPE_TEMP,
    DEV_TYPE_POWER,
    DEV_TYPE_E2PROM,
    DEV_TYPE_FLASH,
    DEV_TYPE_RESET,
    DEV_TYPE_UPGRADE,
    DEV_TYPE_PHY,

    /* UNKONW_TYPE */
};


enum EDevFuncType
{
    DEV_FUNC_TYPE_NONE,
    DEV_FUNC_TYPE_OA,
    DEV_FUNC_TYPE_OA_PA,
    DEV_FUNC_TYPE_OA_BA,

    DEV_FUNC_TYPE_OCM,

    DEV_FUNC_TYPE_OPS,

    DEV_FUNC_TYPE_OSC,

    DEV_FUNC_TYPE_TDCM,

    DEV_FUNC_TYPE_TEMP,

    DEV_FUNC_TYPE_MEM,

    DEV_FUNC_TYPE_OTDR,

    DEV_FUNC_TYPE_EXTEND,

    DEV_FUNC_TYPE_MUX,

    DEV_FUNC_TYPE_ADC_POSITIVE,

    DEV_FUNC_TYPE_ADC_OPPOSITE,

    DEV_FUNC_TYPE_RESET_WARM,
    DEV_FUNC_TYPE_RESET_COLD,

    DEV_FUNC_TYPE_UPGRADE,

    DEV_FUNC_TYPE_STORE,

    DEV_FUNC_TYPE_LED_SYS,
    DEV_FUNC_TYPE_LED_ALM,
    DEV_FUNC_TYPE_LEN_PWR,
    DEV_FUNC_TYPE_LED_FAN,
    DEV_FUNC_TYPE_WSS,
    DEV_FUNC_TYPE_PHY
};
#endif

enum EPortAlarmType
{
    PORT_ALARM_TYPE_LOS,
#if 0
    PORT_ALARM_TYPE_LOW,
    PORT_ALARM_TYPE_HIGH,
#endif
    PORT_ALARM_TYPE_CNT
};

#ifdef __OLSP__
enum EVoaType
{
    VOA_E1_INTER,   /* E1_interVOA */
    VOA_E1_PADDING, /* E1_PaVOA */
    VOA_OSC,        /* OSC_VOA (IN E1) */
    VOA_E2_INTER,   /* E2_interVOA */
    VOA_E2_PADDING, /* E2_PaddingVOA */
    VOA_OCM,        /* OCM VOA(IN E2) */
    VOA_OSC_TX      /* Rcsv OSC_VOA_TX(IN E2) */
};
#endif

#ifdef __NR1004__
enum EVoaType
{
    VOA_E1_INTER,   /* E1_interVOA */
    VOA_E1_NA,      /* E1_NA */
    VOA_OSC,        /* OSC_VOA_RX (IN E1) */
    VOA_E2_NA,      /* E2_NA */
    VOA_E2_PADDING, /* E2_PaddingVOA */
    VOA_OSC_TX,     /* OSC_VOA_TX(IN E2) */
    VOA_OCM         /* Rcsv */
};
#endif

#if defined __HCF__ || defined __OCS__
enum EVoaType
{
    VOA_E1_INTER,   /* BA_interVOA */
    VOA_E2_INTER,   /* PA_interVOA */
    VOA_E2_PADDING, /* PA_PaddingVOA */
    VOA_OSC,        /* OSC_RX_VOA */
    VOA_NULL,
    VOA_OCM,        /* OCM VOA */
    VOA_E1_PADDING, /* BA_PaddingVOA */
    VOA_OSC_TX      /* Rcsv OSC_VOA_TX(IN E2) */
};
#endif

#ifdef __cplusplus
};
#endif

#endif



