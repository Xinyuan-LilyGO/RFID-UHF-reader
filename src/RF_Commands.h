#ifndef __RF_COMMANDS_H__
#define __RF_COMMANDS_H__

#include "HardwareSerial.h"
#include "RF_Interface.h"

struct RFC_Common
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }

private:
    bool valid, updated;
    void commit(uint8_t type, uint8_t Command, char *data)
    {
        updated = true;
        valid = type == 0x01 ? true : false;
    }
};

struct RFC_Inventory
{
    friend class RFC_Class;

public:
    bool isValid() { return valid; }
    bool isUpdated() const { return updated; }
    Inventory_t GetLabel()
    {
        updated = false;
        return _data;
    }

private:
    bool valid, updated;
    uint8_t _type;
    Inventory_t _data;
    void commit(uint8_t type, uint8_t Command, char *data)
    {
        updated = true;
        valid = (type == 0x02 ? true : false);

        _data.RSSI = data[0];
        _data.PC = data[1] << 8 | data[2];
        memcpy(_data.epc, &data[3], 12);
        _data.CRC = data[15] << 8 | data[16];
    }
};

struct RFC_Information
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }

    uint8_t InfoType()
    {
        updated = false;
        return _data[0];
    }
    char *InfoStr()
    {
        updated = false;
        return &_data[1];
    }

private:
    bool valid, updated;
    uint8_t _type;
    char _data[30];
    void commit(uint8_t type, uint8_t len, char *data)
    {
        updated = true;
        valid = type == 0x01 ? true : false;
        memset(_data, '\0', 30);
        memcpy(_data, data, len);
    }
};

struct RFC_PaPower
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }

    uint16_t PaPower()
    {
        updated = false;
        return _pow;
    }

private:
    bool valid, updated;
    uint8_t _type;
    uint16_t _pow;
    void commit(uint8_t type, uint8_t Command, char *data)
    {
        updated = true;
        if (Command == 0xB6)
        {
            valid = type == 0x01 ? true : false;
        }
        else
        {
            _pow = data[0] << 8 | data[1];
        }
    }
};

struct RFC_Query
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    uint16_t Para()
    {
        updated = false;
        return _para;
    }

private:
    bool valid, updated;
    uint8_t _type;
    uint16_t _para;
    void commit(uint8_t type, uint8_t Command, char *data)
    {
        updated = true;
        if (Command == 0x0E)
            valid = type == 0x01 ? true : false;
        else
            _para = data[0] << 8 | data[1];
    }
};

struct RFC_Region
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    uint8_t Region()
    {
        updated = false;
        return _region;
    }

private:
    bool valid, updated;
    uint8_t _type;
    uint8_t _region;
    void commit(uint8_t type, uint8_t Command, char *data)
    {
        updated = true;
        if (Command == 0x07)
            valid = type == 0x01 ? true : false;
        else
            _region = data[0];
    }
};

struct RFC_RfChannel
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    uint8_t channel()
    {
        updated = false;
        return _rfch;
    }

private:
    bool valid, updated;
    uint8_t _type;
    uint8_t _rfch;
    void commit(uint8_t type, uint8_t Command, char *data)
    {
        updated = true;
        if (Command == 0xAB)
            valid = type == 0x01 ? true : false;
        else
            _rfch = data[0];
    }
};

struct RFC_FHSS
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    bool isON()
    {
        updated = false;
        return on;
    }

private:
    bool valid, updated, on;

    void commit(uint8_t type, uint8_t Command, char *data)
    {
        updated = true;
        valid = type == 0x01 ? true : false;
    }
    void state(bool en)
    {
        on = en ? true : false;
    }
};

struct RFC_Select
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    Select_t parameter()
    {
        updated = false;
        return _data;
    }

private:
    bool valid, updated;
    Select_t _data;
    void commit(uint8_t type, uint8_t Command, char *data)
    {
        updated = true;
        valid = type == 0x01 ? true : false;
        if (Command == 0x0B)
        {
            _data.SelParam = data[0];
            _data.Ptr = data[1] << 24 | data[2] << 16 | data[3] << 8 | data[4];
            _data.MaskLen = data[5];
            for (int i = 0; i < _data.MaskLen >> 3; i++)
            {
                _data.Mask[i] = data[i + 6];
            }
        }
    }
};

struct RFC_LabelMemoryBank
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    LabelStorage_t parameter()
    {
        updated = false;
        return _data;
    }

private:
    bool valid, updated;
    LabelStorage_t _data;
    void commit(uint8_t type, uint8_t Command, char *data, uint16_t Len)
    {
        updated = true;
        valid = type == 0x01 ? true : false;
        if (Command == 0x39)
        {
            _data.PC_EPC_Len = data[0];
            _data.PC = data[1] << 8 | data[2];
            for (int i = 0; i < _data.PC_EPC_Len - 2; i++)
            {
                _data.epc[i] = data[3 + i];
            }

            for (int i = 0; i < Len - _data.PC_EPC_Len; i++)
            {
                _data.Data[i] = data[_data.PC_EPC_Len + i];
            }
        }
    }
};

struct RFC_Label
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    uint8_t *GetEPC()
    {
        updated = false;
        return epc;
    }
    uint16_t PC()
    {
        updated = false;
        return pc;
    }

private:
    bool valid, updated;
    uint16_t pc;
    uint8_t epc[12];
    void commit(uint8_t type, uint8_t Command, char *data)
    {
        updated = true;
        valid = type == 0x01 ? true : false;
        pc = data[1] << 8 | data[2];
        for (int i = 0; i < data[0] - 2; i++)
        {
            epc[i] = data[3 + i];
        }
    }
};

struct RFC_Demodulator
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    DemodulatorParameter_t GetParameter()
    {
        updated = false;
        return para;
    }

private:
    bool valid, updated;
    DemodulatorParameter_t para;
    void commit(uint8_t type, uint8_t Command, char *data)
    {
        updated = true;
        valid = type == 0x01 ? true : false;
        if (Command == 0xF1)
        {
            para.Mixer_G = data[0];
            para.IF_G = data[1];
            para.Thrd = data[2] << 8 | data[3];
        }
    }
};

struct RFC_Test
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    BlockingRFInput_t GetBRFI()
    {
        updated = false;
        return bRF;
    }
    RSSIRFInput_t GetRRFI()
    {
        updated = false;
        return rRF;
    }

private:
    bool valid, updated;
    BlockingRFInput_t bRF;
    RSSIRFInput_t rRF;
    void commit(uint8_t type, uint8_t Command, char *data)
    {
        updated = true;
        valid = type == 0x01 ? true : false;
        if (Command == 0xF2)
        {
            bRF.CH_L = data[0];
            bRF.CH_H = data[1];
            for (int i = 0; i < bRF.CH_H - bRF.CH_L + 1; i++)
            {
                bRF.JMR[i] = data[2 + i];
            }
        }
        else
        {
            rRF.CH_L = data[0];
            rRF.CH_H = data[1];
            for (int i = 0; i < rRF.CH_H - rRF.CH_L + 1; i++)
            {
                rRF.RSSI[i] = data[2 + i];
            }
        }
    }
};

struct RFC_Error
{
    friend class RFC_Class;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    uint8_t ErrorCode()
    {
        updated = false;
        valid = false;
        return errorcode;
    }
    uint8_t *ErrorData()
    {
        updated = false;
        valid = false;
        return errordata;
    }

private:
    bool valid, updated;
    uint8_t errorcode;
    uint8_t errordata[30];
    void commit(uint8_t type, uint16_t len, char *data)
    {
        updated = true;
        valid = type == 0x01 ? true : false;
        errorcode = data[0];
        memcpy(errordata, data, len);
    }
};

class RFC_Class
{
    // public:
private:
    bool waitAckDone();

    void SendFrame(RF_Frame frame);
    String BuildFrame(String data);
    RF_Frame BuildFrame(uint8_t msgType, uint8_t cmdCode);
    RF_Frame BuildFrame(uint8_t msgType, uint8_t cmdCode, uint16_t len, uint8_t *data);

    // String BuildFrame(String msgType, String cmdCode, String[] dataArr);
    HardwareSerial *_serial;
    bool idle = true;
    uint32_t encodedCharCount = 0;
    char rxbuf[50];

public:
    RFC_Class(HardwareSerial *Serial) { _serial = Serial; };
    ~RFC_Class(){};

    void begin();
    bool encode(char c);

    RFC_Common common;
    RFC_Inventory inventory;
    RFC_Information info;
    RFC_PaPower power;
    RFC_Query query;
    RFC_Region region;
    RFC_RfChannel rfch;
    RFC_FHSS fhss;
    RFC_Select select;
    RFC_LabelMemoryBank labelmemory;
    RFC_Label label;
    RFC_Demodulator demodulator;
    RFC_Test test;
    RFC_Error error;

    Inventory_t GetLabelOnce();
    String GetModuleInfoFrame(uint8_t infoType);
    uint16_t GetPaPowerFrame();
    uint16_t GetQueryFrame();
    uint8_t GetRegionFrame();
    uint8_t GetRfChannelFrame();
    Select_t GetSelectParameterFrame();

    // No test
    LabelStorage_t GetLabelMomryFrame(uint32_t Password, uint8_t MemBank, uint16_t SA, uint16_t DL);
    DemodulatorParameter_t GetDemodulatorParameterFrame();
    BlockingRFInput_t GetBlockingRFInput();
    RSSIRFInput_t GetRSSORFInput();

    bool SetGetLabelStart(uint16_t CNT);
    bool SetGetLabelStop();
    bool SetComBaudrate(uint16_t baudrate);
    bool SetComSleep();
    bool SetComAutoSleep(uint8_t min);
    bool SetComAutoIDLE(uint8_t min);
    bool SetComInsertChannel(uint8_t Count, uint8_t *Index);
    bool SetComTransmitContinuousCarrier(bool enable);
    bool SetComIO(uint8_t p1, uint8_t p2, uint8_t p3);

    bool SetPaPowerFrame(uint8_t pow);
    bool SetQueryFrame(uint16_t para);
    bool SetRegionFrame(uint8_t reg);
    bool SetRfChannelFrame(uint8_t ch);
    bool SetFHSSFrame(bool enable);
    bool SetSelectParameterFrame(Select_t para);
    bool SetSelectModeFrame(uint8_t mode);
    // No test
    bool SetLabelMomryFrame(uint32_t Password, uint8_t MemBank, uint16_t SA, uint16_t DL, uint8_t *data);
    bool SetLockLabelMomryFrame(uint32_t Password, uint32_t LD);
    bool SetKillLabelFrame(uint32_t Password);
    bool SetDemodulatorParameterFrame(DemodulatorParameter_t para);

    /*  To Do
    0xE0 NXP ChangeConfig 指令
    0xE1 NXP ReadProtec/Reset ReadProtect 指令
    0xE3 NXP Change EAS 指令
    0xE4 NXP EAS-Alarm 指令
    0xE5/0xE6 Impinj Monza 4 QT 指令
    0xD3/0xD4 BlockPermalock 指令
    */
};

#endif /* __RF_COMMANDS_H__ */