#ifndef WH_ALARM_H
#define WH_ALARM_H
#pragma once
/***************************   Include Files   *******************************/
#include <Arduino.h>
#include "whalarm.h"
#include <AsyncSMS.h>
#include <IWatchdog.h>
#include <stdio.h>
#include <Timers.h>
#include <string.h>
#include <Wire.h>
#include <CRC8.h>
#include <CRC.h> //https://crccalc.com/
//            SDA  SCL
// TwoWire Wire1(PB7, PB6); // Onboard PCF and EEprom AT24C04

/***************************   Function Prototypes   *************************/
void setup(void);
void loop(void);
void setupStm(void);
void readInPhNo(void);
void sendSmsTxt(void);
void readPcf8574(void);
void messageReceived(char *number, char *message);
void testPcfInput(void);
void newSmsRecived(void);
void sendSmsAlarm(void);
void sendEeSmsNo(void);
void retStatusSms(void);
void smsWriteEeprom(void);
void makeSmsTxtMsgSms(u_int8_t dataPcf);
void writeEeprom(int16_t addressI2C, u_int16_t eeAddress,
                 u_int8_t *data, u_int16_t numChars);
void readEeprom(int16_t addressI2C, u_int16_t eeAddress,
                u_int8_t *data, u_int16_t numChars);
void readMagnaAlarm(void);
void eraseUsedEeprom(void);
void SettingInEeprom(void);
void sendStatusDaysReset(void);
u_int8_t bcdToByte(u_int8_t bcdHigh, u_int8_t bcdLow);
void updateClock(void);
void testIndput(void);


#define EE24C04P0 0x50 // Address of 24LC04 "block 0"
#define EE24C04P1 0x51 // Address of 24LC04 "block 1"

extern bool masterSmsNo;
extern bool sendZoneSMS;
extern bool smsSendNow;
extern bool delSms;
extern bool newSms;

extern String smsMsg;

extern u_int8_t h; 
extern u_int8_t m; 
extern u_int8_t s;
extern u_int8_t indputToCheck;
extern u_int8_t dataPcf;
extern u_int8_t dataPcf;
extern u_int8_t zoneDataPcf;
extern u_int16_t dayCounter;
extern u_int16_t smsCount;
extern u_int32_t deleteSmsAfterSend;
extern char recSmsNumber[12];
extern u_int8_t phNo0[17]; // Master no
extern u_int8_t phNo1[17];
extern u_int8_t phNo2[17];
extern u_int8_t phNo3[17];
extern u_int8_t phNo4[17];
extern char henrik[12];
extern char timeBuffer[24];
extern u_int8_t eeSetting[17];
extern char sendSmsMumber[12];
extern char recSmsMessage[30];


#endif



