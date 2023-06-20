#ifndef WH_ALARM_H
#define WH_ALARM_H
/***************************   Include Files   *******************************/
#include <Arduino.h>
#include <IWatchdog.h>
#include <stdio.h>
#include <Timers.h>
#include <AsyncSMS.h>
#include <string.h>
#include <Wire.h>
#include "CRC8.h"
#include "CRC.h" //https://crccalc.com/
#include "whalarm.h"

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

#endif



