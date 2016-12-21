/*
 * A library for controlling a Microchip rn2483 LoRa radio.
 *
 * @Author JP Meijers
 * @Author Nicolas Schteinschraber
 * @Date 18/12/2015
 *
 */

#include "Arduino.h"
#include "msRN2483.h"

/*
  @param serial Needs to be an already opened Stream ({Software/Hardwre}Serial) to write to and read from.
*/
msRN2483::msRN2483(Stream& serial) :
_serial(serial) {
	_serial.setTimeout(2000);
}

void msRN2483::autobaud() {
    _serial.write((byte)0x00);
    _serial.write(0x55);
    _serial.println();
	while (!ackMac("RN2483")) {
		sendCmd("sys get ver");
	}
}

void msRN2483::clearSerialBuffer() {
  while(_serial.available())
    _serial.read();
}

bool msRN2483::ackMac(String ACKSTRING) {
	String ret = getAck();
	if (ret.startsWith(ACKSTRING)) {
		return true;
	} else {
		return false;
	}
}

String msRN2483::getAck() {
	return _serial.readStringUntil('\n');
}

void msRN2483::sendCmd(String cmd) {
	clearSerialBuffer();
	_serial.println(cmd);
}

bool msRN2483::sendCmdCnf(String cmd, String ack) {
	clearSerialBuffer();
	_serial.println(cmd);
	return _serial.readStringUntil('\n').startsWith(ack);
}

void msRN2483::initRadio() {

	String dc = String((100.0  / (msRN2483config.dcycle.toFloat()/3.0)) - 1,0);
	dc.trim();
	msRN2483config.pwridx.trim();
	msRN2483config.dr.trim();
	
	clearSerialBuffer();
	while(!sendCmdCnf("mac reset 868","ok")) {};
	while(!sendCmdCnf("mac set rx2 3 869525000","ok")) {};
	while(!sendCmdCnf("mac set adr off","ok")) {};
	while(!sendCmdCnf("mac set ar off","ok")) {};
	while(!sendCmdCnf("mac set pwridx " + msRN2483config.pwridx,"ok")) {}; // power index: 1 = max, 5=min
	while(!sendCmdCnf("mac set dr " + msRN2483config.dr,"ok")) {}; // data rate: 0 = min, 7=max
	while(!sendCmdCnf("mac set ch dcycle 0 " + dc,"ok")) {};
	while(!sendCmdCnf("mac set ch dcycle 1 " + dc,"ok")) {};
	while(!sendCmdCnf("mac set ch dcycle 2 "+ dc,"ok")) {};
	while(!sendCmdCnf("mac save","ok")) {};
}

bool msRN2483::joinNetwork() {
  
	uint8_t retry_count = 0;
	String joinCmd = "mac join ";
	String receivedData;
	clearSerialBuffer();
	if (msRN2483config.AppEUI != "" and msRN2483config.AppKey != "") {
		joinCmd += "otaa";
		sendCmdCnf("mac set appeui " + msRN2483config.AppEUI,"ok");
		sendCmdCnf("mac set appkey " + msRN2483config.AppKey,"ok");
	} else if (msRN2483config.NwkSKey != "" and msRN2483config.AppSKey != "" and msRN2483config.devAddr != "") {
		joinCmd += "abp";
		sendCmdCnf("mac set nwkskey " + msRN2483config.NwkSKey,"ok");
		sendCmdCnf("mac set appskey " + msRN2483config.AppSKey,"ok");
		sendCmdCnf("mac set devaddr " + msRN2483config.devAddr,"ok");
	}
	sendCmdCnf("mac save","ok");

	_serial.setTimeout(60000);

	for (uint8_t retry_count=0; retry_count<10; retry_count++) {
		;
		if (sendCmdCnf(joinCmd,"ok")) {
			receivedData = getAck();	  
			if (receivedData.startsWith("accepted")) {
				_serial.setTimeout(2000);
				return true;	  
			} else {
				msRN2483config.error = receivedData;
				return false;
			}			
		} else if (receivedData.startsWith("no_free_ch") or receivedData.startsWith("busy")) {
			delay(1000);
		} else if (receivedData.startsWith("silent") or receivedData.startsWith("invalid_param") or receivedData.startsWith("keys_not_init") or receivedData.startsWith("accepted")) {
			msRN2483config.error = receivedData;
			return false;
		}
	  
	}
	msRN2483config.error = "joining network timed out";
	return false;
}

bool msRN2483::sendString(String data, String modus, bool encode) {

	uint8_t busy_count = 0;
	uint8_t retry_count = 0;
	String receivedData;
	clearSerialBuffer();

	for (uint8_t retry_count=0; retry_count<10; retry_count++) {
		Serial.println(msRN2483config.error);
		_serial.print("mac tx " + modus + " 1 ");
		if(encode) {
		  sendEncoded(data);
		} else {
		  _serial.println(data);
		}

		if(ackMac("ok")) {
		  _serial.setTimeout(30000);
		  receivedData = getAck();
		  _serial.setTimeout(2000);

		  if(receivedData.startsWith("mac_tx_ok") or receivedData.startsWith("mac_rx")) {
			msRN2483config.rx = receivedData;
			return true;
		  } else if(receivedData.startsWith("mac_err") or receivedData.startsWith("invalid_data_len")) {
			msRN2483config.error = receivedData;
			return false;
		  } else {
			msRN2483config.error = "unknown response";
			return false;
		  }
		} else if(receivedData.startsWith("invalid_param") or receivedData.startsWith("invalid_data_len")) {
		  msRN2483config.error = receivedData;
		  return false;
		} else if(receivedData.startsWith("not_joined") or receivedData.startsWith("silent") or receivedData.startsWith("frame_counter_err_rejoin_needed") or receivedData.startsWith("mac_paused")) {
			initRadio();
			joinNetwork();
		} else if(receivedData.startsWith("no_free_ch")) {
			delay(1000);
		} else if(receivedData.startsWith("busy")) {
			busy_count++;
			if(busy_count >= 10) {
				initRadio();
				joinNetwork();
			 } else {
				delay(1000);
			 }
		} else {
			//unknown error just wait??
			delay(1000);
		}
	}
	msRN2483config.error = "sending data timed out";
	return false;
}

bool msRN2483::sendBytes(const byte* data, uint8_t size) {
  char msgBuffer[size*2 + 1];

  char buffer[3];
  for (unsigned i=0; i<size; i++)
  {
    sprintf(buffer, "%02X", data[i]);
    memcpy(&msgBuffer[i*2], &buffer, sizeof(buffer));
  }
  String dataToTx(msgBuffer);
  return sendString(dataToTx, "uncnf", false);

}

void msRN2483::sendEncoded(String input) {
	char working;
	char buffer[3];
	for (unsigned i=0; i<input.length(); i++) {
		working = input.charAt(i);
		sprintf(buffer, "%02x", int(working));
		_serial.print(buffer);
	}
	_serial.println();
}

String msRN2483::radioSleep(uint32_t ms) {
  //Values < 100 will result in maximum sleep time
  if (ms < 100) {
    ms = 4294967295;
  }
  sendCmd("sys sleep " + String(ms)); // valid sleep range 100 to 4294967295 ms
  return getAck();
}

void msRN2483::radioWake() {
  autobaud();
}
