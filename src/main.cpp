/*=============================================================================
File Name           :
Project File Number : v 0.07b
Project Name        : SMS alarm Sams� 4 x BWE632
Author              :
Start Date          :
Chip                : STM32f401
Copyright(c) 2022, All Rights Reserved.
-------------------------------------------------------------------------------
Description:
-----------------------------------------------------------------------------*/
/***************************   Include Files   *******************************/
#include "whalarm.h"
/***************************   Defines   *************************************/
#define EE24C04P0 0x50 // Address of 24LC04 "block 0" ligger pt også i whalarm.cpp
#define EE24C04P1 0x51 // Address of 24LC04 "block 1" ligger pt også i whalarm.cpp
#define PCFI2CADR 0x20 // Address of PCF
Timers timer;
/***************************   Flags and bit *********************************/
bool newSms = false;
bool sec1 = false;
bool masterSmsNo = true;
bool smsSendNow = false;
bool delSms = false;
bool sendZoneSMS = false;
bool lastSendZoneSMS = false;
/***************************   sbit ******************************************/
/***************************   Macros   **************************************/
/***************************   Data Types   **********************************/
/***************************   Local Variables   *****************************/
u_int32_t deleteSmsAfterSend = millis();
u_int16_t smsCount = 1;
u_int16_t dayCounter = 0;
u_int8_t dataPcf;
u_int8_t zoneDataPcf;
u_int8_t indputToCheck;
String smsMsg;
char timeBuffer[24];
char recSmsNumber[12];
char sendSmsMumber[12];
char recSmsMessage[30];
char henrik[12] = "+4522360040";
u_int8_t phNo0[17] = {"0"}; // Master no
u_int8_t phNo1[17] = {"1"};
u_int8_t phNo2[17] = {"2"};
u_int8_t phNo3[17] = {"3"};
u_int8_t phNo4[17] = {"4"};
u_int8_t eeSetting[17] = {"5"};
u_int8_t h; 
u_int8_t m; 
u_int8_t s;
/*****************************************************************************/
/***************************   Enum   ****************************************/
/***************************   Constants   ***********************************/
/***************************   Global Variables   ****************************/
/*****************************************************************************/
//            SDA  SCL
TwoWire Wire1(PB7, PB6); // Onboard PCF and EEprom AT24C04
// TwoWire Wire1(PB4, PA8); // add by wire I2C 3
//                     RX   TX
HardwareSerial Serial2(PA3, PA2);
AsyncSMS smsHelper(&Serial1, 115200);
/******************************************************************************
Function name : void setup()
         Type : PRIVATE
Description   : Run after start/reset
Notes :
******************************************************************************/
void setup()
{
   setupStm();
   eraseUsedEeprom(); // press buttom PA0 to erase EEprom or recive @* from "henrik"
   readInPhNo();      // read from EEprom
   smsHelper.init();
   smsHelper.smsReceived = *messageReceived;
   if (phNo0[0] != '+')
   {
      masterSmsNo = false;
   }
//  
   Serial2.print("Start UP mS: ");
   Serial2.println(millis());
   sendStatusDaysReset(); // send sms with number of reset
   smsHelper.readGSMTime(); //dummy  so get time after first ask  
   timer.start(5000); //time in ms
   IWatchdog.begin(10000000); // time in uS = 10 sec.
} // END setup
/******************************************************************************
Function name : void Loop()
         Type : PRIVATE
Description   :
Notes :
******************************************************************************/
void loop()
{
/***************************   Local Variables   *****************************/
static u_int32_t WaitSMStoSend = millis();
   smsHelper.process(); // call in main loop
   if ((smsSendNow) && (millis() - WaitSMStoSend >= 100))
   {
      sendSmsAlarm();
      WaitSMStoSend = millis();
   }
   readPcf8574();
   testPcfInput();
   newSmsRecived();
   if (timer.available()) // just to make sure that Timer lib working after 50 days
   {
      timer.stop();
      IWatchdog.reload();
      timer.start(5000);
   }
   if((millis() - deleteSmsAfterSend >= 60000) && (delSms))
   {
      smsHelper.deleteAllSMS();
      delSms = false;
   }
   if((h == 7) && (m == 10) && (s == 30)) // laves til 2 if s� den p� tid kan sende status til mig om reset mm (h == 7) && (m == 10) && 
   {
      if(eeSetting[1] == '1')
      { 
         sendStatusDaysReset();
         dayCounter = dayCounter + 1;
      }
      if(sendZoneSMS == true)
      {
         dataPcf = zoneDataPcf;
         makeSmsTxtMsgSms(dataPcf);
         smsSendNow = true;
         sendZoneSMS = false;
      }
      s = s + 1; // fake sek so not send more than one "find en anden l�sning ;-)"
   }
} // END loop
/******************************************************************************
Function name : void SetupStm32(void)
         Type :
Description   :
Notes :
******************************************************************************/
void setupStm(void)
{
   Serial2.begin(115200);
   while (!Serial2)
      ;
   Wire1.begin();
   pinMode(PA4, INPUT_PULLUP); // Magna3 alarm
   pinMode(PA0, INPUT_PULLUP); // Reset buttom
   pinMode(PC13, OUTPUT);      // sets the digital pin 13 as output
   digitalWrite(PC13, HIGH);
} // END setupStm
/******************************************************************************
Function name : void sendSmsTxt(void)
         Type :
Description   :
Notes :
******************************************************************************/
void sendSmsTxt(void)
/***************************   Local Variables   *****************************/
{
   //   Serial2.print(millis());
   //   Serial2.print(" - ");
   digitalWrite(PC13, LOW);
   smsCount++;
   smsHelper.send(sendSmsMumber, (char *)smsMsg.c_str(), smsMsg.length());
   //   Serial2.write(sendSmsMumber);
   //   Serial2.println("");
   Serial2.write(smsMsg.c_str(), smsMsg.length());
   Serial2.println("");
   deleteSmsAfterSend = millis(); // set 60 (59) sec. after sms to send
   digitalWrite(PC13, HIGH);
   //   Serial2.println(millis());
} // end
/******************************************************************************
Function name : void readPcf8574(void)
         Type :
Description   :
Notes :
******************************************************************************/
void readPcf8574(void)
{
/***************************   Local Variables   *****************************/
static u_int32_t readInterval = millis();
static u_int8_t read3Times = 0;
static u_int8_t inp[3] = {0xFF, 0xFF, 0xFF};
static u_int8_t i = 0;
static u_int8_t dataPcfRead = 0;
   if (sec1)
   {
      if (millis() - readInterval >= 3) // read in 3 times to get 2 correct
      {
         readInterval = millis();
         Wire1.beginTransmission(PCFI2CADR);
         Wire1.endTransmission();
         Wire1.requestFrom(PCFI2CADR, 1); // 1 byte from PCF
      }
      if (Wire1.available())
      {
         dataPcfRead = Wire1.read();
         read3Times++;
         inp[i++] = dataPcfRead;
         if (read3Times >= 3)
         {
            dataPcf = (inp[0] & inp[1]) & inp[2];
            i = 0;
            sec1 = false;
            read3Times = 0;
            readMagnaAlarm();
         }
      }
   }
}

/******************************************************************************
Function name : void testPcfInput(void)
         Type : FSM
Description   :
Notes :
******************************************************************************/
void testPcfInput(void)
{
/***************************   Local Variables   *****************************/
static u_int32_t ReadPCFinterval = 0;
u_int32_t mili_time = 0;
static u_int8_t dataPCFold = 0b11111111;
static u_int8_t LastDataSend = 0b11111111;
static u_int32_t Readcount = 0;
#define DEBOUNCE_TIME 10 // in Sec.
static bool WaitForData = false;
   //
   if (WaitForData)
   {
      WaitForData = false;
      if (dataPcf != dataPCFold)
      {
         if (dataPcf == LastDataSend)
         {
            Readcount = DEBOUNCE_TIME;
         }
         else
         {
            Readcount = 0;
         }
         dataPCFold = dataPcf;
      }
      else if (Readcount < DEBOUNCE_TIME)
      {
         Readcount++;
         if (Readcount == DEBOUNCE_TIME)
         {
            //makeSmsTxtMsgSms(dataPcf); // moved to void testIndput(void)
            indputToCheck = dataPcf;
            testIndput();
            LastDataSend = dataPcf;
         }
      }
   }
   else // WaitForData = false
   {
      mili_time = millis();
      if (mili_time - ReadPCFinterval >= 1000)
      {
         ReadPCFinterval = mili_time;
         // Read interval 1000 = 1 sek.
         updateClock();
         sec1 = true;
         WaitForData = true;
      }
   }
}

/******************************************************************************
Function name : void writeEeprom(int16_t addressI2C, u_int16_t eeAddress,
                                  u_int8_t* data, u_int16_t numChars)
         Type :
Description   :
Notes :
******************************************************************************/
void writeEeprom(int16_t addressI2C, u_int16_t eeAddress,
                 u_int8_t *data, u_int16_t numChars)
{
/***************************   Local Variables   *****************************/   
u_int8_t i = 0;
   Wire1.beginTransmission(addressI2C);
   Wire1.write(eeAddress);
   Wire1.write(data, numChars);
   Wire1.endTransmission(true);
   delay(5); // Not the right solution. but since it is only written when changing the user
   Serial2.println("Write");
}
/***************************************************************************************/

/******************************************************************************************/
/*
src\whalarm.cpp: In function 'void updateClock()':
src\whalarm.cpp:444:7: error: 'smsHelper' was not declared in this scope
  444 |       smsHelper.readGSMTime();
*/
/******************************************************************************
Function name : void updateClock(void)
         Type :
Description   :
Notes :
******************************************************************************/
void updateClock(void)
{
static u_int32_t updateTheTime = millis() + 59000; // update time faster on boot ;-)
u_int8_t bcdHigh;
u_int8_t bcdLow;   
   if((millis() - updateTheTime >= 60000))
   {     
      updateTheTime = millis();
      smsHelper.readGSMTime();
      Serial2.println("update");
      Serial2.println(timeBuffer);
      bcdHigh = (timeBuffer[10] - 48); // Timer
      bcdLow = (timeBuffer[11] - 48); 
      h = bcdToByte(bcdHigh, bcdLow);
      bcdHigh = (timeBuffer[13] - 48); // Minutter
      bcdLow = (timeBuffer[14] - 48); 
      m = bcdToByte(bcdHigh, bcdLow) + 1; // first ask after reset are + 1 min
      bcdHigh = (timeBuffer[16] - 48); // Sekunder
      bcdLow = (timeBuffer[17] - 48); 
      s = bcdToByte(bcdHigh, bcdLow) + 1; // Time is - 1 sek from my pc ;-)
      bcdHigh = (timeBuffer[1] - 48); // Year
      bcdLow = (timeBuffer[2] - 48); 
      u_int8_t y = bcdToByte(bcdHigh, bcdLow);
      if(y < 20) // test if time error on gsm then dont update time is 04 in sim800 if not update time from gsm
      {
         Serial2.println("update error");
      }
   }
      else
      {
            s++;
         if (s >= 60) 
         {
               m++;
               s = 0;
         }
         if (m >= 60) 
         {
               h++;
               m = 0;
         }
         if ((h > 23) || (h == 0)) 
         {
               h = 0;
         }
         Serial2.printf("%01d:%01d:%01d\n", h, m, s); // test print out i 1 sec.
      }
}
/*********************************************************************************************/
/*
src\whalarm.cpp: In function 'void readEeprom(int16_t, u_int16_t, u_int8_t*, u_int16_t)':
src\whalarm.cpp:442:4: error: 'Wire1' was not declared in this scope; did you mean 'Wire'?
  442 |    Wire1.beginTransmission(addressI2C);
      |    ^~~~~
      |    Wire
*/
/******************************************************************************
Function name : void readEeprom(int16_t addressI2C, u_int16_t eeAddress,
                                 u_int8_t* data, u_int16_t numChars)
         Type :
Description   :
Notes :
******************************************************************************/
void readEeprom(int16_t addressI2C, u_int16_t eeAddress,
                u_int8_t *data, u_int16_t numChars)
{
/***************************   Local Variables   *****************************/   
u_int8_t i = 0;
   Wire1.beginTransmission(addressI2C);
   Wire1.write(eeAddress); // LSB in 24c04 only one address byte use addressI2C 0x51 for "page 2"
   Wire1.endTransmission();
   Wire1.requestFrom(addressI2C, numChars);
   while (Wire1.available())
   {
      data[i++] = Wire1.read(); // recive all byte in rxbuffer
   }
}