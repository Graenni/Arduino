/*
	msUbloxGPS.cpp - Library for polling minimal GPS data from an ublox NEO6M GPS module.
	Created by M. Schmutz, December 9, 2016, Basel, Switzerland.
*/


#include "Arduino.h"
#include "msUbloxGPS.h"

msUbloxGPS::msUbloxGPS(Stream& serial):
_gpsSerial(serial)
{
  _gpsSerial.setTimeout(2000);
}

void msUbloxGPS::calcChecksum(byte ubxMessage[], byte ubxMessageLength)
{
  for (int i = 2; i < ubxMessageLength - 2; i++) {
    ubxMessage[ubxMessageLength - 2] += ubxMessage[i];
    ubxMessage[ubxMessageLength - 1] += ubxMessage[ubxMessageLength - 2];
  }
}


boolean msUbloxGPS::writeGPS(byte ubxMessage[], byte ubxMessageSize) {
  for (byte i = 0; i < ubxMessageSize; i++) {
    _gpsSerial.write(ubxMessage[i]);
  }
  return true;
}

boolean msUbloxGPS::configGPS() {
  byte mid[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
  for (byte i = 0; i < sizeof(mid); i++) {
    byte message[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, mid[i], 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    byte messageLength = sizeof(message);
    calcChecksum(message, sizeof(message));
    boolean success = false;
    while (!success) {
      writeGPS(message, messageLength);
      success = getACK(message);
    }
  }
  return true;
}

boolean msUbloxGPS::getACK(byte* ubxMessage) {
  uint32_t startTime = millis();
  byte ackByteID = 0;
  byte ack[] = {0xB5, 0x62, 0x05, 0x01, 0x02, 0x00, ubxMessage[2], ubxMessage[3], 0x00, 0x00};
  calcChecksum(ack, sizeof(ack));
  while ((millis() - startTime) < serialTimeout) {
    if (_gpsSerial.available()) {
      byte c = _gpsSerial.read();
      if (ackByteID == (sizeof(ack) - 1)) {
        return true;
      } else if (c == ack[ackByteID]) {
        ackByteID++;
      } else {
        ackByteID = 0;
      }
    }
  }
  return false;
}

boolean msUbloxGPS::pollNAV(byte msgID, byte* structure, byte structureSize) {
  uint32_t startTime = millis();
  byte message[] = { 0xB5, 0x62, 0x01, msgID, 0x00, 0x00, 0x00, 0x00 };
  byte messageLength = sizeof(message);
  calcChecksum(message, sizeof(message));
  boolean success = false;
  while (!success and (millis() - startTime) < serialTimeout) {
    writeGPS(message, messageLength);
    success = getMSG(structure, structureSize);
  }
  return true;
}

boolean msUbloxGPS::getMSG(byte structure[], byte structureSize) {
  uint32_t startTime = millis();
  byte ackByteID = 0;
  byte CKA;
  byte CKB;

  while ((millis() - startTime) < serialTimeout) {
    if (_gpsSerial.available()) {
      byte c = _gpsSerial.read();

      if (ackByteID < 2) {
        if ( c == structure[ackByteID] )
          ackByteID++;
        else
          ackByteID = 0;
      } else {
        if ( ackByteID < (structureSize - 2) ) {
          structure[ackByteID] = c;
          structure[structureSize - 2] += structure[ackByteID];
          structure[structureSize - 1] += structure[structureSize - 2];
        } else if ( ackByteID == (structureSize - 2) ) {
          CKA = c;
        } else if ( ackByteID == (structureSize - 1) ) {
          CKB = c;
          if ( CKA == structure[structureSize - 2] and CKB == structure[structureSize - 1] ) {
            return true;
          } else {
            return false;
          }
        }
        ackByteID++;
      }
    }
  }
  return false;
}