#include <NeoSWSerial.h>
#define _hardwareSerial Serial

uint8_t RX_PIN = 13;
uint8_t TX_PIN = 12;

NeoSWSerial _softwareSerial(RX_PIN, TX_PIN);

void setup()
{
  _hardwareSerial.begin(9600);
  _softwareSerial.begin(9600);
  delay(1000);
}


void loop()
{
  if (_hardwareSerial.available()) {
    while (_hardwareSerial.available()) {
      _softwareSerial.write(_hardwareSerial.read());
    }
  }
  if (_softwareSerial.available()) {
    while (_softwareSerial.available()) {
      _hardwareSerial.write(_softwareSerial.read());
    }
  }
}

