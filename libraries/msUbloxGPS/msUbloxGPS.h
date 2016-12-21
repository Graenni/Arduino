/*
	msUbloxGPS.h - Library for polling minimal GPS data from an ublox NEO6M GPS module.
	Created by M. Schmutz, December 9, 2016, Basel, Switzerland.
*/

#ifndef msUbloxGPS_h
#define msUbloxGPS_h

#include "Arduino.h"

struct NAV_POSLLH {
		  byte sych1 = 0xB5;
		  byte sych2 = 0x62;
		  byte cls = 0;
		  byte id = 0;
		  unsigned short len = 0;
		  uint32_t iTOW = 0;
		  int32_t lon = 0;
		  int32_t lat = 0;
		  int32_t height = 0;
		  int32_t hMSL = 0;
		  uint32_t hAcc = 0;
		  uint32_t vAcc = 0;
		  byte cka = 0;
		  byte ckb = 0;
};

struct NAV_STATUS {
		  byte sych1 = 0xB5;
		  byte sych2 = 0x62;
		  byte cls = 0;
		  byte id = 0;
		  unsigned short len = 0;
		  uint32_t iTOW = 0;
		  byte gpsFix = 0;
		  byte flags = 0;
		  byte fixStat = 0;
		  byte flags2 = 0;
		  uint32_t ttff = 0;
		  uint32_t msss = 0;
		  byte cka = 0;
		  byte ckb = 0;
};

struct NAV_DOP {
		  byte sych1 = 0xB5;
		  byte sych2 = 0x62;
		  byte cls = 0;
		  byte id = 0;
		  unsigned short len = 0;
		  uint32_t iTOW = 0;
		  uint16_t gDOP = 0;
		  uint16_t pDOP = 0;
		  uint16_t tDOP = 0;
		  uint16_t vDOP = 0;
		  uint16_t hDOP = 0;
		  uint16_t nDOP = 0;
		  uint16_t eDOP = 0;
};

class msUbloxGPS
{	
	public:
		
		/*  */
		msUbloxGPS(Stream& serial);
			
		/* public functions */
		boolean configGPS();
		boolean pollNAV(byte msgID, byte* structure, byte structureSize);
	
	private:
		
		/* private functions */
		void calcChecksum(byte ubxMessage[], byte ubxMessageLength);
		boolean writeGPS(byte ubxMessage[], byte ubxMessageSize);
		boolean getACK(byte* ubxMessage);
		boolean getMSG(byte structure[], byte structureSize);
		
		/* private variables */
		Stream& _gpsSerial;
		uint16_t serialTimeout = 3000;

};

#endif