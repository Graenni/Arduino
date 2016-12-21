/*
 * A library for controlling a Microchip RN2xx3 LoRa radio.
 *
 * @Author JP Meijers
 * @Author Nicolas Schteinschraber
 * @Date 18/12/2015
 *
 */

#ifndef msRN2483_h
#define msRN2483_h

#include "Arduino.h"

struct MSRN2483CONFIG {
	String AppEUI = "";
	String AppKey = "";
	String NwkSKey = "";
	String AppSKey = "";
	String devAddr = "";
	String error = "";
	String rx = "";
	String pwridx = "1";
	String dr = "0";
	String dcycle = "1";
};


class msRN2483
{
  public:

	MSRN2483CONFIG msRN2483config;
  
    msRN2483(Stream& serial);
	void autobaud();
	String getAck();
	void sendCmd(String cmd);
	bool sendCmdCnf(String cmd, String ack);
	void initRadio();
	bool joinNetwork();
	bool sendString(String data, String modus, bool encode);
	bool sendBytes(const byte* data, uint8_t size);
	String radioSleep(uint32_t ms);
	void radioWake();  
	
  private:
    Stream& _serial;
	void clearSerialBuffer();
	bool ackMac(String ACKSTRING);
    void sendEncoded(String input);
    
};

#endif
