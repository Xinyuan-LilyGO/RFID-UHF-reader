#include "RF_Commands.h"

void RFC_Class::begin()
{
}

bool RFC_Class::waitAckDone()
{
    uint32_t Millis = millis();
    while (!_serial->available())
    {
        if (millis() - Millis > 100)
            return false;
    }
    uint32_t count = _serial->available(); //判断字符是否50us内无新发送
    while (count)
    {
        delayMicroseconds(50);
        if (count < _serial->available())
        {
            count = _serial->available();
        }
        else
        {
            uint8_t count = _serial->available();
            while (count--)
            {
                encode(_serial->read());
            }
            return true;
        }
    }
}

void RFC_Class::SendFrame(RF_Frame frame)
{
    RF_Data data;
    uint16_t len = (frame.PL_H << 8) + frame.PL_L;
    data.frame = frame;
    _serial->write(data.data, len + 5);
    // Serial.write(data.data, len + 5);
    _serial->write(data.frame.Checksum);
    // Serial.write(data.frame.Checksum);
    _serial->write(data.frame.End);
    // Serial.write(data.frame.End);
}

String RFC_Class::BuildFrame(String data)
{
}

RF_Frame RFC_Class::BuildFrame(uint8_t msgType, uint8_t cmdCode)
{
    RF_Frame frame;
    frame.Header = 0xBB;
    frame.Type = msgType;
    frame.Command = cmdCode;
    frame.PL_H = 0x00;
    frame.PL_L = 0x00;
    frame.Checksum = (frame.Type + frame.Command) & 0xFF;
    frame.End = 0x7E;
    return frame;
}

RF_Frame RFC_Class::BuildFrame(uint8_t msgType, uint8_t cmdCode, uint16_t len, uint8_t *data)
{
    RF_Frame frame;
    uint16_t ver = 0;
    uint8_t *dataPtr = data;
    frame.Header = 0xBB;
    frame.Type = msgType;
    frame.Command = cmdCode;
    frame.PL_H = len >> 8;
    frame.PL_L = len & 0xFF;
    memcpy(frame.Parameter, data, len);
    for (int i = 0; i < len; i++)
    {
        ver += *dataPtr;
        // Serial.printf("%02X\n", *dataPtr);
        dataPtr++;
    }
    // Serial.printf("%02X\n", ver);
    frame.Checksum = (frame.Type + frame.Command + frame.PL_H + frame.PL_L + ver) & 0xFF;
    frame.End = 0x7E;
    return frame;
}

bool RFC_Class::encode(char c)
{
    encodedCharCount++;
    if (idle)
    {
        if (c == 0xBB)
        {
            idle = false;
            encodedCharCount = 0;
            rxbuf[encodedCharCount] = c;
            // Serial.printf("encodedCharCount = 0\n");
        }
    }
    else
    {
        rxbuf[encodedCharCount] = c;
        if (encodedCharCount > 5)
        {
            uint16_t len = (rxbuf[PL_H] << 8) | rxbuf[PL_L];
            if (rxbuf[len + 6] == 0x7E)
            {

                // verify
                uint16_t ver = 0;
                for (int i = 1; i < len + 5; i++)
                {
                    ver += rxbuf[i];
                }
                ver = ver & 0xFF;
                if (ver == rxbuf[len + 5])
                {
                    // Serial.printf("ver Success\n");
                    //解析
                    switch (rxbuf[Command])
                    {
                    case 0x03:
                        info.commit(rxbuf[Type], rxbuf[PL_H] << 8 | rxbuf[PL_L], &rxbuf[Parameter]);
                        break;
                    case 0x22: // once
                    case 0x27: // mult
                        inventory.commit(rxbuf[Type], rxbuf[Command], &rxbuf[Parameter]);
                        break;

                    case 0xB6:
                    case 0xB7:
                        power.commit(rxbuf[Type], rxbuf[Command], &rxbuf[Parameter]);
                        break;

                    case 0x0E:
                    case 0x0D:
                        query.commit(rxbuf[Type], rxbuf[Command], &rxbuf[Parameter]);
                        break;

                    case 0x07:
                    case 0x08:
                        region.commit(rxbuf[Type], rxbuf[Command], &rxbuf[Parameter]);
                        break;

                    case 0xAA:
                    case 0xAB:
                        rfch.commit(rxbuf[Type], rxbuf[Command], &rxbuf[Parameter]);
                        break;

                    case 0xAD:
                        fhss.commit(rxbuf[Type], rxbuf[Command], &rxbuf[Parameter]);
                        break;

                    case 0x0C:
                    case 0x0B:
                    case 0x12:
                        select.commit(rxbuf[Type], rxbuf[Command], &rxbuf[Parameter]);
                        break;

                    case 0x04:
                    case 0x11:
                    case 0x17:
                    case 0x1D:
                    case 0xA9:
                    case 0xB0:
                        common.commit(rxbuf[Type], rxbuf[Command], &rxbuf[Parameter]);
                        break;

                    case 0x39:
                        labelmemory.commit(rxbuf[Type], rxbuf[Command], &rxbuf[Parameter], rxbuf[PL_H] << 8 | rxbuf[PL_L]);
                        break;

                    case 0x65:
                    case 0x82:
                        label.commit(rxbuf[Type], rxbuf[Command], &rxbuf[Parameter]);
                        break;

                    case 0xF0:
                    case 0xF1:
                        demodulator.commit(rxbuf[Type], rxbuf[Command], &rxbuf[Parameter]);
                        break;

                    case 0xFF:
                        /*                        Serial.print("Error Code : 0x");
                                               Serial.println(rxbuf[Parameter], HEX); */
                        error.commit(rxbuf[Type], rxbuf[PL_H] << 8 | rxbuf[PL_L], &rxbuf[Parameter]);
                        break;

                    default:

                        break;
                    }

                    memset(rxbuf, '\0', sizeof(rxbuf));
                }
                idle = true;
            }
        }
    }
    if (idle == false && encodedCharCount > 30)
    {
        idle = true;
        Serial.println("data error!");
    }

    return false;
}

String RFC_Class::GetModuleInfoFrame(uint8_t infoType)
{
    uint8_t Parameter = infoType;
    SendFrame(BuildFrame(0X00, 0X03, 1, &Parameter));
    if (!waitAckDone())
    {
        return "Error : No Data.\n";
    }
    uint32_t Millis = millis();
    while (!info.isUpdated())
    {
        if (millis() - Millis > 100)
            return "Error";
    }
    return String(info.InfoStr());
}

uint16_t RFC_Class::GetPaPowerFrame()
{
    SendFrame(BuildFrame(0X00, 0XB7));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!power.isUpdated())
    {
        if (millis() - Millis > 100)
            return 0;
    }
    return power.PaPower();
}

bool RFC_Class::SetPaPowerFrame(uint8_t pow)
{
    uint8_t data[2];
    switch (pow)
    {
    case 0:
        data[0] = 0x07;
        data[1] = 0xD0;
        break;
    case 1:
        data[0] = 0x07;
        data[1] = 0x3A;
        break;
    case 2:
        data[0] = 0x06;
        data[1] = 0xA4;
        break;
    case 3:
        data[0] = 0x06;
        data[1] = 0x0E;
        break;
    case 4:
        data[0] = 0x05;
        data[1] = 0x78;
        break;
    case 5:
        data[0] = 0x04;
        data[1] = 0xE2;
        break;
    default:
        data[0] = 0x07;
        data[1] = 0xD0;
        break;
    }
    SendFrame(BuildFrame(0X00, 0XB6, 2, data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!power.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return power.isValid();
}

uint16_t RFC_Class::GetQueryFrame()
{
    SendFrame(BuildFrame(0X00, 0X0D));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!query.isUpdated())
    {
        if (millis() - Millis > 100)
            return 0;
    }
    return query.Para();
}

bool RFC_Class::SetQueryFrame(uint16_t para)
{
    uint8_t data[2];

    data[0] = para >> 8;
    data[1] = para & 0xff;

    SendFrame(BuildFrame(0X00, 0X0E, 2, data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!query.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return query.isValid();
}

uint8_t RFC_Class::GetRegionFrame()
{
    SendFrame(BuildFrame(0X00, 0X08));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!region.isUpdated())
    {
        if (millis() - Millis > 100)
            return 0;
    }
    return region.Region();
}

bool RFC_Class::SetRegionFrame(uint8_t reg)
{
    uint8_t data;

    data = reg;

    SendFrame(BuildFrame(0X00, 0X07, 1, &data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!region.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return region.isValid();
}
/**
 * @description:
 * 中国900MHz 信道参数计算公式，Freq_CH 为信道频率：
 * Freq_CH = CH_Index * 0.25M + 920.125M
 * 中国800MHz 信道参数计算公式，Freq_CH 为信道频率：
 * Freq_CH = CH_Index * 0.25M + 840.125M
 * 美国信道参数计算公式，Freq_CH 为信道频率：
 * Freq_CH = CH_Index * 0.5M + 902.25M
 * 欧洲信道参数计算公式，Freq_CH 为信道频率：
 * Freq_CH = CH_Index * 0.2M + 865.1M
 * 韩国信道参数计算公式，Freq_CH 为信道频率：
 * Freq_CH = CH_Index * 0.2M + 917.1M
 * @param {*}
 * @return {*}
 */
uint8_t RFC_Class::GetRfChannelFrame()
{
    SendFrame(BuildFrame(0X00, 0XAA));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!rfch.isUpdated())
    {
        if (millis() - Millis > 100)
            return 0;
    }
    return rfch.channel();
}
/**
 * @description: 设置信道
 * 中国900MHz 信道参数计算公式，Freq_CH 为信道频率：
 * CH_Index = (Freq_CH-920.125M)/0.25M
 * 中国800MHz 信道参数计算公式，Freq_CH 为信道频率：
 * CH_Index = (Freq_CH-840.125M)/0.25M
 * 美国信道参数计算公式，Freq_CH 为信道频率：
 * CH_Index = (Freq_CH-902.25M)/0.5M
 * 欧洲信道参数计算公式，Freq_CH 为信道频率：
 * CH_Index = (Freq_CH-865.1M)/0.2M
 * 韩国信道参数计算公式，Freq_CH 为信道频率：
 * CH_Index = (Freq_CH-917.1M)/0.2M
 * @param {uint8_t} reg
 * @return {*}
 */
bool RFC_Class::SetRfChannelFrame(uint8_t reg)
{
    uint8_t data;

    data = reg;

    SendFrame(BuildFrame(0X00, 0XAB, 1, &data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!rfch.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return rfch.isValid();
}

bool RFC_Class::SetFHSSFrame(bool enable)
{
    uint8_t data;
    data = enable ? 0xFF : 0x00;
    SendFrame(BuildFrame(0X00, 0XAD, 1, &data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!fhss.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    fhss.state(enable);
    return fhss.isValid();
}

Select_t RFC_Class::GetSelectParameterFrame()
{
    SendFrame(BuildFrame(0X00, 0X0B));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        // return 0;
    }
    uint32_t Millis = millis();
    while (!select.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return select.parameter();
}

bool RFC_Class::SetSelectParameterFrame(Select_t para)
{
    uint8_t data[30];
    uint8_t i = 0;
    data[i++] = para.SelParam;
    data[i++] = (para.Ptr >> 24) & 0xFF;
    data[i++] = (para.Ptr >> 16) & 0xFF;
    data[i++] = (para.Ptr >> 8) & 0xFF;
    data[i++] = para.Ptr & 0xFF;
    data[i++] = para.MaskLen;
    data[i++] = para.Truncate;
    for (int j = 0; j < para.MaskLen >> 3; j++)
    {
        data[i + j] = para.Mask[j];
    }
    SendFrame(BuildFrame(0X00, 0X0C, 7 + para.MaskLen >> 3, data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!select.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return select.isValid();
}

bool RFC_Class::SetSelectModeFrame(uint8_t mode)
{
    uint8_t data;
    data = mode;
    SendFrame(BuildFrame(0X00, 0X12, 1, &data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!select.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return select.isValid();
}

bool RFC_Class::SetComBaudrate(uint16_t baudrate)
{
    uint8_t data[2];
    data[0] = baudrate >> 8;
    data[1] = baudrate & 0XFF;
    SendFrame(BuildFrame(0X00, 0X11, 1, data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!common.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return common.isValid();
}

bool RFC_Class::SetComSleep()
{
    SendFrame(BuildFrame(0X00, 0X17));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!common.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return common.isValid();
}

bool RFC_Class::SetComAutoSleep(uint8_t min)
{
    uint8_t data;
    data = min;
    SendFrame(BuildFrame(0X00, 0X1D, 1, &data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!common.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return common.isValid();
}

bool RFC_Class::SetComAutoIDLE(uint8_t min)
{
    uint8_t data[2];
    data[0] = min;
    data[1] = 0x01;
    SendFrame(BuildFrame(0X00, 0X04, 2, data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!common.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return common.isValid();
}

LabelStorage_t RFC_Class::GetLabelMomryFrame(uint32_t Password, uint8_t MemBank, uint16_t SA, uint16_t DL)
{
    uint8_t data[10];
    data[0] = (Password >> 24) & 0XFF;
    data[1] = (Password >> 16) & 0XFF;
    data[2] = (Password >> 8) & 0XFF;
    data[3] = Password & 0XFF;

    data[4] = MemBank;

    data[5] = (SA >> 8) & 0XFF;
    data[6] = SA & 0XFF;

    data[7] = (DL >> 8) & 0XFF;
    data[8] = DL & 0XFF;
    SendFrame(BuildFrame(0X00, 0X39, 9, data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        // return 0;
    }
    uint32_t Millis = millis();
    while (!labelmemory.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return labelmemory.parameter();
}

bool RFC_Class::SetLabelMomryFrame(uint32_t Password, uint8_t MemBank, uint16_t SA, uint16_t DL, uint8_t *data)
{
    uint8_t _data[30];
    _data[0] = (Password >> 24) & 0XFF;
    _data[1] = (Password >> 16) & 0XFF;
    _data[2] = (Password >> 8) & 0XFF;
    _data[3] = Password & 0XFF;

    _data[4] = MemBank;

    _data[5] = (SA >> 8) & 0XFF;
    _data[6] = SA & 0XFF;

    _data[7] = (DL >> 8) & 0XFF;
    _data[8] = DL & 0XFF;
    for (int i = 0; i < DL << 2; i++)
    {
        _data[9 + i] = data[i];
    }
    SendFrame(BuildFrame(0X00, 0X49, 9 + DL << 2, _data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        // return 0;
    }
    uint32_t Millis = millis();
    while (!labelmemory.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return labelmemory.isValid();
}

bool RFC_Class::SetLockLabelMomryFrame(uint32_t Password, uint32_t LD)
{
    uint8_t _data[10];
    _data[0] = (Password >> 24) & 0XFF;
    _data[1] = (Password >> 16) & 0XFF;
    _data[2] = (Password >> 8) & 0XFF;
    _data[3] = Password & 0XFF;

    _data[4] = (LD << 16) & 0XFF;
    _data[5] = (LD >> 8) & 0XFF;
    _data[6] = LD & 0XFF;

    SendFrame(BuildFrame(0X00, 0x82, 7, _data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!label.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return label.isValid();
}

bool RFC_Class::SetKillLabelFrame(uint32_t Password)
{
    uint8_t _data[4];
    _data[0] = (Password >> 24) & 0XFF;
    _data[1] = (Password >> 16) & 0XFF;
    _data[2] = (Password >> 8) & 0XFF;
    _data[3] = Password & 0XFF;

    SendFrame(BuildFrame(0X00, 0x65, 4, _data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!label.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return label.isValid();
}

bool RFC_Class::SetComInsertChannel(uint8_t Count, uint8_t *Index)
{
    uint8_t _data[20];
    _data[0] = Count;
    for (int i = 0; i < Count; i++)
    {
        _data[1 + i] = Index[i];
    }
    SendFrame(BuildFrame(0X00, 0xA9, Count + 1, _data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!common.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return common.isValid();
}

bool RFC_Class::SetComTransmitContinuousCarrier(bool enable)
{
    uint8_t data;
    data = enable ? 0xFF : 0x00;
    SendFrame(BuildFrame(0X00, 0XB0, 1, &data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!common.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return common.isValid();
}

DemodulatorParameter_t RFC_Class::GetDemodulatorParameterFrame()
{
    SendFrame(BuildFrame(0X00, 0XF1));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        // return 0;
    }
    uint32_t Millis = millis();
    while (!demodulator.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return demodulator.GetParameter();
}

bool RFC_Class::SetDemodulatorParameterFrame(DemodulatorParameter_t para)
{
    uint8_t data[4];
    uint8_t i = 0;
    data[i++] = para.Mixer_G;

    data[i++] = para.IF_G;

    data[i++] = (para.Thrd >> 8) & 0xff;
    data[i++] = para.Thrd & 0xff;

    SendFrame(BuildFrame(0X00, 0XF0, 4, data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!demodulator.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return demodulator.isValid();
}

BlockingRFInput_t RFC_Class::GetBlockingRFInput()
{
    SendFrame(BuildFrame(0X00, 0XF2));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        // return 0;
    }
    uint32_t Millis = millis();
    while (!test.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return test.GetBRFI();
}

RSSIRFInput_t RFC_Class::GetRSSORFInput()
{
    SendFrame(BuildFrame(0X00, 0XF3));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        // return 0;
    }
    uint32_t Millis = millis();
    while (!test.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return test.GetRRFI();
}

bool RFC_Class::SetComIO(uint8_t p1, uint8_t p2, uint8_t p3)
{
    uint8_t data[3];
    data[0] = p1;
    data[1] = p2;
    data[2] = p3;
    SendFrame(BuildFrame(0X00, 0X1A, 3, data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
        return 0;
    }
    uint32_t Millis = millis();
    while (!common.isUpdated())
    {
        if (millis() - Millis > 100)
            return false;
    }
    return common.isValid();
}

Inventory_t RFC_Class::GetLabelOnce()
{
    
    SendFrame(BuildFrame(0X00, 0X22));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
    }
    uint32_t Millis = millis();
    while (!inventory.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return inventory.GetLabel();
}

bool RFC_Class::SetGetLabelStart(uint16_t CNT)
{
    uint8_t data[3];
    data[0] = 0x22;
    data[1] = CNT >> 8;
    data[2] = CNT & 0xff;
    SendFrame(BuildFrame(0X00, 0X27, 3, data));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
    }
    uint32_t Millis = millis();
    while (!inventory.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return inventory.isValid();
}

bool RFC_Class::SetGetLabelStop()
{
    SendFrame(BuildFrame(0X00, 0X28));
    if (!waitAckDone())
    {
        Serial.println("Error : No Data.");
    }
    uint32_t Millis = millis();
    while (!inventory.isUpdated())
    {
        if (millis() - Millis > 100)
            break;
    }
    return inventory.isValid();
}