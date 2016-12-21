/*
 * Library for TvB LoRaWAN Node Rev.1
 * 
 *
 * Thomas van Bellegem - 2016
 * thomas.van.bellegem@hotmail.com
*/

#include "TvBLoRaWANNodeRev1.h"
#include "Arduino.h"

TvBLoRaWANNodeRev1::TvBLoRaWANNodeRev1() {
}


// Initialize hardware
void TvBLoRaWANNodeRev1::Init() {
 
 #define LED_ONBOARD 31
 #define LED_RED 21
 #define LED_GREEN 22
 #define LED_BLUE 23
 #define VoltagePin	A6
 
 Serial.begin(115200);
 analogReference(INTERNAL1V1);
 
 pinMode(LED_ONBOARD, OUTPUT);
 pinMode(LED_RED, OUTPUT);
 pinMode(LED_GREEN, OUTPUT);
 pinMode(LED_BLUE, OUTPUT);

 digitalWrite(LED_RED, HIGH);
 digitalWrite(LED_GREEN, HIGH);
 digitalWrite(LED_BLUE, HIGH);
 
}


// Initialize module with the RFM9X configuration
void TvBLoRaWANNodeRev1::Init_RFM95W() {
 
 #define LORAWAN_NSS 4
 #define LORAWAN_DIO0 18
 #define LORAWAN_DIO1 19
 #define LORAWAN_DIO2 20
 
}

// Initialize module with the RN2483 configuration
void TvBLoRaWANNodeRev1::Init_RN2483() { 
 
 #define RN2483_RESET 15
 Serial1.begin(57600);	// Default baudrate for RN2483 serial communication
 pinMode(RN2483_RESET, OUTPUT);

}



// TvBWirelessNode.Voltage();
// Voltage divider to measure input (VIN) voltage
float TvBLoRaWANNodeRev1::Voltage() {
  // Measure voltage in "x.xx" volts
  analogRead(VoltagePin);
  delay(250);
  int sensorValue = analogRead(VoltagePin);
  delay(50);
  float voltage1 = (sensorValue * 1.1) / 1000;
  float voltage2 = voltage1 * ((470/100) + 1);
  return voltage2;
}