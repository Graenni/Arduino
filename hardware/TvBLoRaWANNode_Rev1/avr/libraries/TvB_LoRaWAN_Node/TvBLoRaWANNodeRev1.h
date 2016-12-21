/*
 * Library for TvB LoRaWAN Node Rev.1
 * 
 *
 * Thomas van Bellegem - 2016
 * thomas.van.bellegem@hotmail.com
*/

#ifndef TvBLoRaWANNodeRev1_h
#define TvBLoRaWANNodeRev1_h

#include "Arduino.h"


// TvB LoRaWAN Node config
 #define LED_ONBOARD 31
 #define LED_RED 21
 #define LED_GREEN 22
 #define LED_BLUE 23
 #define VoltagePin	A6
 
 // Using RFM9X version
 #define LORAWAN_NSS 4
 #define LORAWAN_DIO0 18
 #define LORAWAN_DIO1 19
 #define LORAWAN_DIO2 20
 
 // Using RN2483 version
 #define RN2483_RESET 15
 


// Functions
class TvBLoRaWANNodeRev1 {
  public:
    TvBLoRaWANNodeRev1();
    
    // Setup
    void Init();
    void Init_RFM95W();
    void Init_RN2483();
    float Voltage();
    

  private:
};

#endif
