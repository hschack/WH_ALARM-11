#include "whalarm.h"

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

/******************************************************************************
Function name : byte bcdToByte(byte bcdHigh, byte bcdLow)
         Type :
Description   :
Notes :
******************************************************************************/
u_int8_t bcdToByte(u_int8_t bcdHigh, u_int8_t bcdLow) 
{
   u_int8_t result = (bcdHigh * 10) + bcdLow;
   return result;
}
/******************************************************************************
Function name : void testIndput(void)
         Type :
Description   :
Notes :
******************************************************************************/
void testIndput(void)
{
#define ZONEMASK 0b00001111
#define ALARMMASK 0b11110000   
static u_int8_t zone1to4;
static u_int8_t lastZone1to4;
static u_int8_t alarmInp;
static u_int8_t lastAlarmInp;
static u_int8_t lastSendZoneStatus = 0b00001111;

   alarmInp = indputToCheck & ALARMMASK;
   zone1to4 = indputToCheck & ZONEMASK;
   
   if(alarmInp != lastAlarmInp)
   {
      makeSmsTxtMsgSms(dataPcf);
      smsSendNow = true; 
      lastAlarmInp = alarmInp;        
   }
   if(zone1to4 != lastZone1to4)
   {
      lastZone1to4 = zone1to4;
   }
   if((lastZone1to4 != lastSendZoneStatus) && (sendZoneSMS == false))
   {
      sendZoneSMS = true;
      Serial2.println(lastSendZoneStatus);
      zoneDataPcf = dataPcf & 0b10001111; // marsk magna and fase 1 2 3 
      lastSendZoneStatus = lastZone1to4;
   }
}
/******************************************************************************
Function name : void sendSmsAlarm(void)
         Type : FSM
Description   : sendSmsTxt();
Notes :
******************************************************************************/
void sendSmsAlarm(void)
{
/***************************   Local Variables   *****************************/
static u_int8_t sendAlarmState = 0;
   switch (sendAlarmState)
   {
   case 0:
      smsSendNow = true;
      // Move to next state
      deleteSmsAfterSend = millis(); // set 15 (14) sec. after first to sms to send
      sendAlarmState = 1;
      break;
   case 1:
      if (phNo0[0] == '+') // send always to master if exist
      {
         strncpy(sendSmsMumber, (char *)phNo0, 11);
         sendSmsTxt();
      }
      // Move to next state
      sendAlarmState = 2;
      break;
   case 2:
      if (phNo1[13] == '1') // to phone no 1
      {
         strncpy(sendSmsMumber, (char *)phNo1, 11);
         sendSmsTxt();
      }
      // Move to next state
      sendAlarmState = 3;
      break;
   case 3:
      if (phNo2[13] == '1') // to phone no 2
      {
         strncpy(sendSmsMumber, (char *)phNo2, 11);
         sendSmsTxt();
      }
      // Move to next state
      sendAlarmState = 4;
      break;
   case 4:
      if (phNo3[13] == '1') // to phone no 3
      {
         strncpy(sendSmsMumber, (char *)phNo3, 11);
         sendSmsTxt();
      }
      // Move to next state
      sendAlarmState = 5;
      break;
   case 5:
      if (phNo4[13] == '1') // to phone no 4
      {
         strncpy(sendSmsMumber, (char *)phNo4, 11);
         sendSmsTxt();
      }
      // Move to next state
      sendAlarmState = 6;
      break;
   case 6:
      smsSendNow = false;
      // Move to next state      
      sendAlarmState = 0; // reset to state 0
      delSms = true;
      break;
   default:
      break;
   }
}
/******************************************************************************
Function name : void sendEeSmsNo(void)
         Type :
Description   :
Notes :
******************************************************************************/
void sendEeSmsNo(void)
{
   if ((strcmp(recSmsNumber, henrik) == 0) ||
       (strcmp(recSmsNumber, (char *)phNo0) == 0)) // equal 0 ;-)
   {
      smsMsg = "Aktive Numre\n";
      smsMsg += (char *)phNo0;
      smsMsg += ("\n");
      smsMsg += (char *)phNo1;
      smsMsg += ("\n");
      smsMsg += (char *)phNo2;
      smsMsg += ("\n");
      smsMsg += (char *)phNo3;
      smsMsg += ("\n");
      smsMsg += (char *)phNo4;
      strncpy(sendSmsMumber, recSmsNumber ,11);
      sendSmsTxt();
   }
}
/******************************************************************************
Function name : void retStatusSms(void)
         Type :
Description   :
Notes :
******************************************************************************/
void retStatusSms(void)
{
   if ((strncmp(recSmsNumber, (char *)phNo0, 11) == 0) ||
       (strncmp(recSmsNumber, (char *)phNo1, 11) == 0) ||
       (strncmp(recSmsNumber, (char *)phNo2, 11) == 0) ||
       (strncmp(recSmsNumber, (char *)phNo3, 11) == 0) ||
       (strncmp(recSmsNumber, (char *)phNo4, 11) == 0)) // equal 0 ;-)
   {
      makeSmsTxtMsgSms(dataPcf);
      strncpy(sendSmsMumber, recSmsNumber ,11);
      sendSmsTxt();
      delSms = true;
      deleteSmsAfterSend = millis(); // set 60 (59) sec. after sms to send
   }
}
/******************************************************************************
Function name : void makeSmsTxtMsgSms(u_int8_t dataPcf)
         Type :
Description   :
Notes :
******************************************************************************/
void makeSmsTxtMsgSms(u_int8_t dataPcf)
{
   smsMsg = "Samsoe 4 x BWE 632\n";
   smsMsg += "Zone 1: ";    // Zone 1
   if (bitRead(dataPcf, 0)) // test Zone 1 for low pellets
   {
      smsMsg += "OK\n";
   }
   else
   {
      smsMsg += "LAV\n";
   }
   smsMsg += "Zone 2: ";    // Zone 2
   if (bitRead(dataPcf, 1)) // test Zone 2 for low pellets
   {
      smsMsg += "OK\n";
   }
   else
   {
      smsMsg += "LAV\n";
   }
   smsMsg += "Zone 3: ";    // Zone 3
   if (bitRead(dataPcf, 2)) // test Zone 3 for low pellets
   {
      smsMsg += "OK\n";
   }
   else
   {
      smsMsg += "LAV\n";
   }
   smsMsg += "Zone 4: ";    // Zone 4
   if (bitRead(dataPcf, 3)) // test Zone 4 for low pellets
   {
      smsMsg += "OK\n";
   }
   else
   {
      smsMsg += "LAV\n";
   }
   smsMsg += "Fase 1: ";    // Fase 1
   if (bitRead(dataPcf, 4)) // test Fase 1 for power
   {
      smsMsg += "ALARM\n";
   }
   else
   {
      smsMsg += "OK\n";
   }
   smsMsg += "Fase 2: ";    // Fase 2
   if (bitRead(dataPcf, 5)) // test Fase 2 for power
   {
      smsMsg += "ALARM\n";
   }
   else
   {
      smsMsg += "OK\n";
   }
   smsMsg += "Fase 3: ";    // Fase 3
   if (bitRead(dataPcf, 6)) // test Fase 3 for power
   {
      smsMsg += "ALARM\n";
   }
   else
   {
      smsMsg += "OK\n";
   }
   smsMsg += "Magna3: "; // Test Magna pump for ERROR
   if (bitRead(dataPcf, 7))
   {
      smsMsg += "OK\n";
   }
   else
   {
      smsMsg += "ALARM\n";
   }
} // end check input
/******************************************************************************
Function name : void readMagnaAlarm(void)
         Type :
Description   :
Notes :
******************************************************************************/
void readMagnaAlarm(void)
{
   bitWrite(dataPcf, 7, digitalRead(PA4)); // set alarm on LOW
}
/******************************************************************************
Function name : void sendStatusDaysReset(void)
         Type :
Description   :
Notes :
******************************************************************************/
void sendStatusDaysReset(void)
{
   smsMsg = "Antal dage i drift: ";
   smsMsg += dayCounter;
   smsMsg += "\n";
   smsMsg += "Antal Reset: ";
   smsMsg += eeSetting[0];
   smsMsg += "\n";
   strncpy(sendSmsMumber, henrik ,11);
   sendSmsTxt();
}
/******************************************************************************
Function name : void messageReceived(char * number, char * message)
         Type :
Description   :
Notes :
*******************************************************************************/
void messageReceived(char *number, char *message)
{
   // Do something with your message
   Serial2.println("Message received");
   Serial2.println(number);
   Serial2.println(message);
   strncpy(recSmsNumber, number ,11);
   strncpy(recSmsMessage, message, 20);
   newSms = true;
}
/******************************************************************************
Function name : void newSmsRecived(void)
         Type :
Description   :
Notes :
******************************************************************************/
void newSmsRecived(void)
{
   if (newSms)
   {
      if (recSmsMessage[0] == '#') // set new numers in EEprom and ram
      {
         smsWriteEeprom();
      }
      else if (recSmsMessage[0] == '?') // get status send back to asking number
      {
         retStatusSms();
      }
      else if (recSmsMessage[0] == '$') // send number and setting back to master and "henrik"
      {
         sendEeSmsNo();
      }
      if ((masterSmsNo == false) && (strncmp(recSmsNumber, recSmsMessage, 11) == 0)) // set the master number after reset or new device
      {
         u_int8_t *data;
         strncpy((char *)phNo0, recSmsMessage, 11);
         data = (uint8_t *)&phNo0[0];
         phNo0[15] = crc8(data, 15, 0x07, 0x00, 0x00, false, false);
         writeEeprom(EE24C04P0, 0x00, phNo0, 16);
         masterSmsNo = true;
      }
      if ((recSmsMessage[0] == '@') && (recSmsNumber, (char *)henrik, 11)) // check number == "henrik" in Function and pin & *
      {
         SettingInEeprom();
      }
      newSms = false;
   }
}
/******************************************************************************
Function name : void
         Type :
Description   :
Notes :
******************************************************************************/
/******************************************************************************
Function name : void SettingInEeprom(void)
         Type :
Description   :
Notes :
******************************************************************************/
void SettingInEeprom(void)
{
   if (recSmsMessage[1] == 'C')
   {
      eeSetting[1] = recSmsMessage[2]; // do not save in EEprom 
   }
   if (recSmsMessage[1] == '*')
   {
      eraseUsedEeprom();
   }
   if (recSmsMessage[1] == 'R')
   {
   NVIC_SystemReset();
   }
}
/******************************************************************************
Function name : void eraseUsedEeprom(void)
         Type :
Description   :
Notes : // if buttom preset on boot in 5 sec.
******************************************************************************/
void eraseUsedEeprom(void)
{
/***************************   Local Variables   *****************************/   
char DeleteEE[17] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
u_int8_t *data;
   if ((digitalRead(PA0) == LOW) || (recSmsMessage[0] == '@'))
   {
      digitalWrite(PC13, LOW);
      delay(5000); // test buttom press > 5 sec.
      if ((digitalRead(PA0) == LOW) || (recSmsMessage[0] == '@'))
      {
         strncpy((char *)phNo0, DeleteEE, 16);
         data = (uint8_t *)&phNo0[0];
         phNo0[15] = crc8(data, 15, 0x07, 0x00, 0x00, false, false);
         writeEeprom(EE24C04P0, 0x00, phNo0, 16);

         strncpy((char *)phNo1, DeleteEE, 16);
         data = (uint8_t *)&phNo1[0];
         phNo1[15] = crc8(data, 15, 0x07, 0x00, 0x00, false, false);
         writeEeprom(EE24C04P0, 0x10, phNo1, 16);

         strncpy((char *)phNo2, DeleteEE, 16);
         data = (uint8_t *)&phNo2[0];
         phNo2[15] = crc8(data, 15, 0x07, 0x00, 0x00, false, false);
         writeEeprom(EE24C04P0, 0x20, phNo2, 16);

         strncpy((char *)phNo3, DeleteEE, 16);
         data = (uint8_t *)&phNo3[0];
         phNo3[15] = crc8(data, 15, 0x07, 0x00, 0x00, false, false);
         writeEeprom(EE24C04P0, 0x30, phNo3, 16);

         strncpy((char *)phNo4, DeleteEE, 16);
         data = (uint8_t *)&phNo4[0];
         phNo4[15] = crc8(data, 15, 0x07, 0x00, 0x00, false, false);
         writeEeprom(EE24C04P0, 0x40, phNo4, 16);

         strncpy((char *)eeSetting, DeleteEE, 16);
         // eeSetting[0] = 0x00; // start with 1 after boot (255 = 0 on first boot)
         writeEeprom(EE24C04P1, 0x10, eeSetting, 16);
      }
      digitalWrite(PC13, HIGH);
   }
}
/******************************************************************************
Function name : void readInPhNo(void)
         Type :
Description   :
Notes :
******************************************************************************/
void readInPhNo(void)
{
/***************************   Local Variables   *****************************/   
uint8_t *data;
u_int8_t eeReadError = 0;
   readEeprom(EE24C04P0, 0x00, phNo0, 16); // master no
   data = (uint8_t *)&phNo0[0];
   if (phNo0[15] != crc8(data, 15, 0x07, 0x00, 0x00, false, false))
   {
      eeReadError = eeReadError + 1;
   }
   readEeprom(EE24C04P0, 0x10, phNo1, 16); // Phone no 1
   data = (uint8_t *)&phNo1[0];
   if (phNo1[15] != crc8(data, 15, 0x07, 0x00, 0x00, false, false))
   {
      eeReadError = eeReadError + 3;
   }
   readEeprom(EE24C04P0, 0x20, phNo2, 16); // Phone no 2
   data = (uint8_t *)&phNo2[0];
   if (phNo2[15] != crc8(data, 15, 0x07, 0x00, 0x00, false, false))
   {
      eeReadError = eeReadError + 5;
   }
   readEeprom(EE24C04P0, 0x30, phNo3, 16); // Phone no 3
   data = (uint8_t *)&phNo3[0];
   if (phNo3[15] != crc8(data, 15, 0x07, 0x00, 0x00, false, false))
   {
      eeReadError = eeReadError + 11;
   }
   readEeprom(EE24C04P0, 0x40, phNo4, 16); // Phone no 4
   data = (uint8_t *)&phNo4[0];
   if (phNo4[15] != crc8(data, 15, 0x07, 0x00, 0x00, false, false))
   {
      eeReadError = eeReadError + 12;
   }
   readEeprom(EE24C04P1, 0x10, eeSetting, 16); // Setting from EEprom
   eeSetting[0] = eeSetting[0] + 1;
   eeSetting[1] = '1';
   writeEeprom(EE24C04P1, 0x10, eeSetting, 16);
   Serial2.print("CRC8 Error: ");
   Serial2.println(eeReadError);
   if (eeReadError != 0)
   {
      smsMsg = "EEprom read ERROR!\n";
      smsMsg += eeReadError;
      smsMsg += "\n";
      strncpy(sendSmsMumber, henrik ,11);
      sendSmsTxt();
   }
}
/******************************************************************************
Function name : void smsWriteEeprom(void)
         Type :
Description   :
Notes : https://crccalc.com/
******************************************************************************/
void smsWriteEeprom(void)
{
/***************************   Local Variables   *****************************/
u_int8_t *data;
   if ((strcmp(recSmsNumber, henrik) == 0) ||
       (strcmp(recSmsNumber, (char *)phNo0) == 0)) // equal 0 ;-) from master or "henrik"
   {
      if (recSmsMessage[13] == '1')
      {
         strncpy((char *)phNo1, recSmsMessage + 1, 16);
         data = (uint8_t *)&phNo1[0];
         phNo1[15] = crc8(data, 15, 0x07, 0x00, 0x00, false, false);
         writeEeprom(EE24C04P0, 0x10, phNo1, 16);
      }
      else if (recSmsMessage[13] == '2')
      {
         strncpy((char *)phNo2, recSmsMessage + 1, 16);
         data = (uint8_t *)&phNo2[0];
         phNo2[15] = crc8(data, 15, 0x07, 0x00, 0x00, false, false);
         writeEeprom(EE24C04P0, 0x20, phNo2, 16);
      }
      else if (recSmsMessage[13] == '3')
      {
         strncpy((char *)phNo3, recSmsMessage + 1, 16);
         data = (uint8_t *)&phNo3[0];
         phNo3[15] = crc8(data, 15, 0x07, 0x00, 0x00, false, false);
         writeEeprom(EE24C04P0, 0x30, phNo3, 16);
      }
      else if (recSmsMessage[13] == '4')
      {
         strncpy((char *)phNo4, recSmsMessage + 1, 16);
         data = (uint8_t *)&phNo4[0];
         phNo4[15] = crc8(data, 15, 0x07, 0x00, 0x00, false, false);
         writeEeprom(EE24C04P0, 0x40, phNo4, 16);
      }
   }
}

