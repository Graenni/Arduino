
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   P R E A M B L E
*/

/* INCLUDE LIBRARIES */
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <NeoSWSerial.h>
#include <msUbloxGPS.h>
#include <msRN2483.h>

/* DEFINITIONS */
#define _rn2483Serial Serial1
//#define _debugSerial Serial

/* DECLARATIONS */
byte NAVSTATUS_msgID = 0x03;
byte NAVPOSLLH_msgID = 0x02;
byte NAVDOP_msgID = 0x04;
boolean validGPS = false;
boolean led_init = false;
const int32_t interval = 20;
uint8_t RN2483_RST_PIN = 16;
uint8_t GPS_RX_PIN = 13;
uint8_t GPS_TX_PIN = 12;
uint8_t STATUS_LED_GND = 0;
uint8_t STATUS_LED = 1;

/* FIRE LIBRARIES */
NeoSWSerial _gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
msUbloxGPS gps(_gpsSerial);
msRN2483 radio(_rn2483Serial);

/* WATCHDOG INTERRUPT */
ISR (WDT_vect)
{
  wdt_disable();  // disable watchdog
}  // end of WDT_vect


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   I N I T I A L   S E T U P
*/
void setup()
{

  //output LED pin
  led_state(1);

  // start debug serial communication
#ifdef _debugSerial
  _debugSerial.begin(9600);
  _debugSerial.println("# # # Startup # # #");
#endif

  // start serial communication with RN2483
  _rn2483Serial.begin(57600); //serial port to radio
#ifdef _debugSerial
  _debugSerial.println("> initializing RN2483 module:");
#endif

  //setup config radio moduel config structure
  radio.msRN2483config.NwkSKey = "C0AA06CB2ACA1D64C047E772F210A836";
  radio.msRN2483config.AppSKey = "623C1CC8BB020152D607CD8F7F8FC83B";
  radio.msRN2483config.devAddr = "26011725";
  radio.msRN2483config.pwridx = "1";
  radio.msRN2483config.dr = "5";
  radio.msRN2483config.dcycle = "30";

  //hard reset rn2483
  pinMode(RN2483_RST_PIN, OUTPUT);
  digitalWrite(RN2483_RST_PIN, LOW);
  delay(500);
  digitalWrite(RN2483_RST_PIN, HIGH);

  radio.autobaud(); //Autobaud the rn2483 module to 9600. The default would otherwise be 57600.
  radio.initRadio(); //apply radio seetings

  //join network
  bool join_result = false;
  while (!join_result) {
    join_result = radio.joinNetwork();

#ifdef _debugSerial
    _debugSerial.println("  - joining network: " + String(join_result));
#endif
  }

 /*radio.sendString("MS-TTN-MAPPER > STARTUP MESSAGE", "uncnf", true);
#ifdef _debugSerial
  _debugSerial.println("  - sending startup message: " + radio.msRN2483config.rx);
#endif*/

  //print out the HWEUI so that we can register it via ttnctl
#ifdef _debugSerial
  radio.sendCmd("sys get hweui");
  _debugSerial.println("  - when using OTAA, register this DevEUI: " + radio.getAck());
  radio.sendCmd("sys get ver");
  _debugSerial.println("  - RN2483 firmware version: " + radio.getAck());
#endif

  radio.radioSleep(0);

#ifdef _debugSerial
  _debugSerial.println("  - radio setup ok");
#endif

#ifdef _debugSerial
  _debugSerial.println("> initializing GPS module:");
#endif

  // start serial communication with GPS
  _gpsSerial.begin(9600);
  if (gps.configGPS()) {
#ifdef _debugSerial
    _debugSerial.println("  - GPS setup ok");
#endif
  }

#ifdef _debugSerial
  _debugSerial.println("# # # # # # # # # #");
#endif

  led_state(0);

  delay(1000);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   M A I N   L O O P
*/
void loop()
{

  led_state(1);

#ifdef _debugSerial
  _debugSerial.println("> polling GPS data:");
#endif

  NAV_STATUS navstatus;
  NAV_POSLLH navposllh;
  NAV_DOP navdop;

  validGPS = gps.pollNAV(NAVSTATUS_msgID, (byte*)&navstatus, sizeof(navstatus));

  if (validGPS and (navstatus.gpsFix == 0x02 or navstatus.gpsFix == 0x03)) {
    validGPS = gps.pollNAV(NAVPOSLLH_msgID, (byte*)&navposllh, sizeof(navposllh));
    if (validGPS) {
      gps.pollNAV(NAVDOP_msgID, (byte*)&navdop, sizeof(navdop));
#ifdef _debugSerial
      _debugSerial.print("  - gpsFix: " + String(navstatus.gpsFix));
      _debugSerial.print(", lat: " + String(navposllh.lat / 10000000.0, 6));
      _debugSerial.print(", lon: " + String(navposllh.lon / 10000000.0, 6));
      _debugSerial.print(", height: " + String(navposllh.height / 1000.0, 6));
      _debugSerial.print(", hMSL: " + String(navposllh.hMSL / 1000.0, 6));
      _debugSerial.print(", hAcc: " + String(navposllh.hAcc / 1000.0, 6));
      _debugSerial.print(", vAcc: " + String(navposllh.vAcc / 1000.0, 6));
      _debugSerial.println(", hdop: " + String(navdop.hDOP / 100.0, 6));
      _debugSerial.println("> TXing:");
#endif

      uint32_t LatitudeBinary = ((navposllh.lat / 10000000.0 + 90) / 180) * 16777215;
      uint32_t LongitudeBinary = ((navposllh.lon / 10000000.0 + 180) / 360) * 16777215;
      uint16_t altitudeGPS = navposllh.height / 1000.0;
      uint8_t hdopGPS = navdop.hDOP / 10;
      uint8_t txBuffer[9];

      txBuffer[0] = ( LatitudeBinary >> 16 ) & 0xFF;
      txBuffer[1] = ( LatitudeBinary >> 8 ) & 0xFF;
      txBuffer[2] = LatitudeBinary & 0xFF;
      txBuffer[3] = ( LongitudeBinary >> 16 ) & 0xFF;
      txBuffer[4] = ( LongitudeBinary >> 8 ) & 0xFF;
      txBuffer[5] = LongitudeBinary & 0xFF;
      txBuffer[6] = ( altitudeGPS >> 8 ) & 0xFF;
      txBuffer[7] = altitudeGPS & 0xFF;
      txBuffer[8] = hdopGPS & 0xFF;

      /*String payload = String(navposllh.lat / 10.0, 0) + " " + String(navposllh.lon / 10.0, 0) + " " + String(navposllh.hMSL / 1000.0, 0) + " " + String(navdop.hDOP, 6); //52307267 4946829 28 80 where the first two are latitude and longitude in millions of degrees, followed by altitude in meters, and then hdop*100
        radio.sendData(payload, "uncnf", true);*/

      radio.radioWake();
      radio.sendBytes(txBuffer, sizeof(txBuffer));
      radio.radioSleep(0);

#ifdef _debugSerial
      _debugSerial.println("  - " + radio.msRN2483config.rx);
#endif
      led_state(0);
      if (radio.msRN2483config.rx.startsWith("mac_tx")) {
        blinkLed(2, 500);
      } else if (radio.msRN2483config.rx.startsWith("mac_rx")) {
        blinkLed(3, 500);
      } else {
        blinkLed(5, 50);
      }
    }
  } else {
#ifdef _debugSerial
    Serial.println("  - no valid gpsFix: " + String(navstatus.gpsFix));
#endif
    validGPS = false;
    led_state(0);
    blinkLed(5, 50);
  }

  for (int i = 0; i < interval; i++) {
    cpuSleep(); // sleep for 1 second
    if (!validGPS) {
      blinkLed(2, 50);
    } else {
      if (i % 3 == 0) {
        blinkLed(1, 50);
      }
    }
  }

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    U S E R   F U N C T I O N S
*/

/* TURN ON/OFF LED */
void led_state(int state) {
  if (!led_init) {
    pinMode(STATUS_LED_GND, OUTPUT);
    digitalWrite(STATUS_LED_GND, 0);
    pinMode(STATUS_LED, OUTPUT);
    led_init = true;
  }
  digitalWrite(STATUS_LED, state);

}

void blinkLed(uint8_t n, uint16_t t) {
  for (uint8_t i = 0; i < n; i++) {
    delay(t);
    led_state(1);
    delay(t);
    led_state(0);
  }
}

/* SEND CPU TO SLEEP MODE */
void cpuSleep() {

#ifdef _debugSerial
  _debugSerial.flush();
#endif

  // clear various "reset" flags
  MCUSR = 0;
  // allow changes, disable reset
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval
  WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1);    // set WDIE, and 1 second delay
  wdt_reset();  // pat the dog

  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  cli();           // timed sequence follows
  sleep_enable();
  sleep_bod_disable(); // turn off brown-out enable in software
  sei();
  sleep_cpu();

  // cancel sleep as a precaution
  sleep_disable();
}
