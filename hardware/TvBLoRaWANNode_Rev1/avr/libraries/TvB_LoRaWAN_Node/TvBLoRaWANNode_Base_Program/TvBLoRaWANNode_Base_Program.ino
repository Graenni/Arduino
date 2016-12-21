/*
 * TvB LoRaWAN Node Rev.1 for RFM95W or RN2483 modules
 * BATT LED:
 *  - orange ON:     charging
 *  - orange FLASH:  no battery connected / tricle charging
 *  - OFF:           fully charged or running on battery
 * 
 * 
 * Basic Hardware:
 *  Extra Blue LED 31
 *  RGB LED: R: 21 (Common Anode)
 *           G: 22 (Common Anode)
 *           B: 23 (Common Anode)
 *  Batt voltage measurement pin A6
 * 
 * 
 * RFM95W version:
 *  Default hardware SPI on pins 4 (CS), 5(MOSI), 6(MISO) & 7(SCK)
 *  DIO0: 18
 *  DIO1: 19
 *  DIO2: 20
 * 
 * 
 * RN2483 version:
 *  RX/TX lines on harware serial port 1 (Serial1), default baudrate 57600
 *  RN2483 reset pin: 15
 *  
 */


#include <TvBLoRaWANNodeRev1.h>

TvBLoRaWANNodeRev1 TvBLoRaWANNodeRev1;

// Setup
void setup() {
  TvBLoRaWANNodeRev1.Init();  // Init the basic node hardware
  
  //TvBLoRaWANNodeRev1.Init_RFM95W(); // Init the RFM95W module: Serial1 & reset pin (does not include LoRaWAN library functionalities yet)
  //TvBLoRaWANNodeRev1.Init_RN2483(); // Init the RN2483 module: hardware SPI & DIO0, 1 & 2 pins (does not include LoRaWAN library functionalities yet)

  digitalWrite(LED_GREEN, LOW); // Green LED ON
  Serial.print(TvBLoRaWANNodeRev1.Voltage()); // Measure & display battery voltage on serial port
  Serial.println(" BATT Voltage (only correct when 3.7V battery connected)");  
  delay(1000);
  digitalWrite(LED_GREEN, HIGH); // Green LED OFF
}


// Main
void loop() {

  // Just a RGB loop here
  digitalWrite(LED_RED, LOW); // Red LED ON
  delay(1000);
  digitalWrite(LED_RED, HIGH); // Red LED OFF
  digitalWrite(LED_GREEN, LOW); // Green LED ON
  delay(1000);
  digitalWrite(LED_GREEN, HIGH); // Green LED OFF
  digitalWrite(LED_BLUE, LOW); // Blue LED ON
  delay(1000);
  digitalWrite(LED_BLUE, HIGH); // Blue LED ON

}