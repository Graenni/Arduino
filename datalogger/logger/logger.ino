#include <SPI.h>
#include <SD.h>
#include "Wire.h"
#include <dht.h>


/* # # # RTC
 * wire SCL:SCL, SDA:SDA, VCC:VCC, GND:GND
 */
#define DS3231_I2C_ADDRESS 0x68
byte decToBcd(byte val){return( (val/10*16) + (val%10) );}
byte bcdToDec(byte val){return( (val/16*10) + (val%16) );}

/* # # # SD CARD
 * wire CS:8, DI:11, VCC:VCC, SCK:13, GND:GND, D0:12
 */
const int chipSelect = 4;
const int fileSize = 10; //number of records in ring memory
File dataFile; //file handle

/* # # # DATA
 * DHT: wire VCC:VCC, SIGNAL:3, GND:GND 
 */
#define DHT11_PIN 3
dht DHT;
struct datastore {
  byte second;
  byte minute;
  byte hour;
  byte dayOfWeek;
  byte dayOfMonth;
  byte month;
  byte year;
  float temp;
  float relh;
};

/* # # # INTERVAL TIMER
 * ... 
 */
unsigned long previousMillis = 0;
const long interval = 1000;

/* # # # SETUP
 * ...
 */
void setup() {
  Serial.begin(9600);
  Serial.print("Initializing card...");
  pinMode(10, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }

  Serial.println("card initialized.");
  
  //create data container
  dataFile = SD.open("bindata");

  Wire.begin();
}

/* # # # MAIN PROGRAM
 * ...
 */
void loop() {

  //check timer
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    //create structure
    struct datastore myData;

    //get timestamp
    readDS3231time(&myData.second, &myData.minute, &myData.hour, &myData.dayOfWeek, &myData.dayOfMonth, &myData.month, &myData.year);
    
    //read voltage and convert
    int chk = DHT.read11(DHT11_PIN);
    myData.temp = DHT.temperature;
    myData.relh = DHT.humidity;
    
    //if max size of ring memory reached, jump bacl to zero
    if (dataFile.position() > sizeof(myData)*fileSize) {dataFile.seek(0);}

    //write data to sd card
    dataFile.write((const uint8_t *)&myData, sizeof(myData));       
  }
}

/* # # # USER FUNCTIONS
 * ...
 */
 
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{ // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
{ // reads time and date data from DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}


