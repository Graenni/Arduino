#include "RTClib.h"
//#include "Wire.h"
#include "TimeLib.h"


RTC_DS3231 RTC;


/* # # # INTERVAL TIMER
 * wire GND:GND, VCC:VCC, SDA:4, SCL:5
 */

byte DS3231_PowerPin = 7; 
int32_t thisTime;
uint32_t timeOffset;
const int32_t interval = 5;
int32_t previousMillis = 0;
int32_t millisOffset;
uint32_t uTime;
uint32_t lastTime = 0;


/* # # # SETUP
 * ...
 */
void setup() {

  pinMode(DS3231_PowerPin, OUTPUT);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {} //FOR DEBUG ONLY
  Serial.flush();
  Serial.println("Serial port ready!");

  digitalWrite(DS3231_PowerPin,HIGH);

  if (! RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1); //DO SOMETHING MORE MEANINGFUL HERE
  }

  if (RTC.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  DateTime now = RTC.now();
  
  setTime(now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());

digitalWrite(DS3231_PowerPin,LOW);
  
}

/* # # # MAIN PROGRAM
 * ...
 */
void loop() {


  uTime = now();

  //check time
  if (uTime - lastTime >= interval and uTime % interval == 0) {
    int t0, t1;
    t0 = millis();
    digitalWrite(DS3231_PowerPin,HIGH);
    DateTime now = RTC.now();
    digitalWrite(DS3231_PowerPin,LOW);

    Serial.print("internal unixtime: ");
    Serial.print(uTime);
    
    Serial.print(" | ds3231 unixtime: ");
    Serial.println(now.unixtime());
  
   setTime(now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());
    
    lastTime = uTime;    
    t1 = millis();
    Serial.println(t1-t0);
  }

}
