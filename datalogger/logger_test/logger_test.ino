#include <SPI.h>
#include "SdFat.h"
#include "TimeLib.h"
#include "RTClib.h"
#include "LowPower.h"

/* # # # INTERVAL TIMER
   wire GND:GND, VCC:VCC, SDA:4, SCL:5
*/
RTC_DS3231 RTC;
byte DS3231_PowerPin = 7;
const int32_t interval = 5;
uint32_t uTime;
uint32_t lastTime = 0;

/* # # # SD CARD
   wire CS:8, MOSI:11, VCC:VCC, CLK:13, GND:GND, MISO:12
*/
byte SD_PowerPin = 6;
const uint32_t fileSize = 100000; //number of records in ring memory
uint32_t filePointer = 0;
const uint16_t chipSelect = 2;
SdFat sd;
SdFile dataFile;
#define error(msg) sd.errorHalt(F(msg));

/* # # # DATA
   DHT: wire VCC:VCC, SIGNAL:3, GND:GND
*/
byte tempPin = 3;
uint32_t recordnumber = 0;
float Vin = 3.3;
float R1 = 10000;
struct datastore {
  uint32_t unixts;
  uint32_t recn;
  float temp;
  float relh;
};


/* # # # SETUP
   ...
*/
void setup() {

  pinMode(DS3231_PowerPin, OUTPUT);
  pinMode(SD_PowerPin, OUTPUT);
  digitalWrite(SD_PowerPin, LOW);
 
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  //initialize DS3231
  Serial.print("Initializing RTC...");
  digitalWrite(DS3231_PowerPin, HIGH);

  if (! RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1); //DO SOMETHING MORE MEANINGFUL HERE
  }

  if (RTC.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  DateTime now = RTC.now();
  
  digitalWrite(DS3231_PowerPin, LOW);

  //sync internal time with RTC
  setTime(now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());
  
  Serial.println("RTC initialized.");
  
  Serial.flush();
  
}

/* # # # MAIN PROGRAM
   ...
*/
void loop() {

 //get time from RTC
  digitalWrite(DS3231_PowerPin, HIGH);
  DateTime now = RTC.now();
  digitalWrite(DS3231_PowerPin, LOW);
  uTime = now.unixtime();

  //check time
  if (uTime - lastTime >= interval and uTime % interval == 0) {
    uint32_t t0;
    t0 = millis();

    lastTime = uTime;

    //create data structure
    struct datastore myData;
    myData.unixts = now.unixtime();
    myData.recn = recordnumber;

    //measure temperature
    float raw, R2;
    raw = analogRead(tempPin);
    if (raw) {
      R2 = R1 * ((Vin / ((raw * Vin) / 1024.0)) - 1);
      myData.temp = 0.0101 * pow(log(R2),4) - 0.4294 * pow(log(R2),3) + 7.8115 * pow(log(R2),2) - 84.745 * log(R2) + 372.81;
    }
 
    myData.relh = 42;
    recordnumber++;


    Serial.print("timestamp: " + String(myData.unixts) + " | RN: " + String(myData.recn) + " | T: " + String(myData.temp) + " | rH: " + String(myData.relh));

    digitalWrite(SD_PowerPin, HIGH);

 
    if (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
      Serial.print(" | SD card not available");
      //sd.initErrorHalt();
    } else {
  
      //open file for write
      if (dataFile.open("bindata.dat", O_RDWR | O_CREAT | O_AT_END)) {
      
        //set pointer to write data
        dataFile.seekSet(filePointer);
  
        //write data to sd card
        dataFile.write((const uint8_t *)&myData, sizeof(myData));
        dataFile.close();
        
        filePointer +=  sizeof(myData);
  
        //if max size of ring memory reached, jump back to zero
        if (filePointer >= sizeof(myData)*fileSize) {
          Serial.print(" | max file size reached...starting over ring memory!");
          filePointer = 0;
        }
  
        //Serial.println("done!");
      } else {
         Serial.print(" | error opening file");
      }
    }
   digitalWrite(SD_PowerPin, LOW);

    Serial.println(" | runtime: " + String(millis() - t0) + " ms");
    Serial.flush();
  }
   
  

  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);

}


