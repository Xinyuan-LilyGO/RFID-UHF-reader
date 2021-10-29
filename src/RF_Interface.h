/*
 * @Author: your name
 * @Date: 2021-10-20 10:15:39
 * @LastEditTime: 2021-10-28 17:40:27
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \MagicRF M100\src\RF_Interface.h
 */
#ifndef __RF_INTERFACE_H__
#define __RF_INTERFACE_H__

#include <stdint.h>

enum
{
    Header = 0,
    Type,
    Command,
    PL_H,
    PL_L,
    Parameter,
};

enum
{
    REGION_CODE_CHN1 = 0x01,
    REGION_CODE_US,
    REGION_CODE_EUR,
    REGION_CODE_CHN2,
    REGION_CODE_KOREA = 0x06,
};

struct RF_Frame
{
    uint8_t Header;
    uint8_t Type;
    uint8_t Command;
    uint8_t PL_H;
    uint8_t PL_L;
    char Parameter[50];
    uint8_t Checksum;
    uint8_t End;
};

union RF_Data
{
    RF_Frame frame;
    uint8_t data[57];
};

struct Select_t
{
    uint16_t SelParam;
    uint32_t Ptr;
    uint8_t MaskLen;
    uint8_t Truncate;
    uint8_t Mask[20];
};

struct LabelStorage_t
{
    uint8_t PC_EPC_Len;
    uint16_t PC;
    uint8_t epc[20];
    uint8_t Data[50];
};

struct DemodulatorParameter_t
{
    uint8_t Mixer_G;
    uint8_t IF_G;
    uint16_t Thrd;
};

struct BlockingRFInput_t
{
    uint8_t CH_L;
    uint8_t CH_H;
    int8_t JMR[20];
};

struct RSSIRFInput_t
{
    uint8_t CH_L;
    uint8_t CH_H;
    int8_t RSSI[20];
};

struct Inventory_t
{
    uint8_t RSSI;
    uint16_t PC;
    uint8_t epc[12];
    uint16_t CRC;
};

#endif /* __RF_INTERFACE_H__ */
