/*
 * @Author: your name
 * @Date: 2021-10-19 17:54:59
 * @LastEditTime: 2021-10-29 09:39:37
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \MagicRF M100\src\main.cpp
 */

#include <Arduino.h>
#include "RF_Commands.h"
#include "HardwareSerial.h"

HardwareSerial SerialRF(2);
RFC_Class rfc(&SerialRF);

#define RFIDEN 14

void setup()
{
    pinMode(RFIDEN, OUTPUT);
    digitalWrite(RFIDEN, 0);
    delay(100);
    digitalWrite(RFIDEN, 1);
    delay(500);

    SerialRF.begin(115200, SERIAL_8N1, 2, 13);
    Serial.begin(115200);
    rfc.begin();
}

void loop()
{

    delay(200);

    while (SerialRF.available())
        rfc.encode(SerialRF.read());

    Serial.printf("hardware version : %s\n", rfc.GetModuleInfoFrame(0x00).c_str());
    Serial.printf("software version : %s\n", rfc.GetModuleInfoFrame(0x01).c_str());
    Serial.printf("manufacturer : %s\n", rfc.GetModuleInfoFrame(0x02).c_str());
}