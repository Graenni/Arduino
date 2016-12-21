#include <LoRaWAN.h>

LoRaWAN lora(Serial);

#define SECOND 1000
#define MINUTE SECOND * 60

const int MSG_DELAY = 1 * MINUTE;

void setup() {
  Serial.begin(115200);
  delay(500);
  lora.join();
}

void loop() {
  // put your main code here, to run repeatedly:
  sendLambData(0.4, 133, 0xABBA);
  
}

void sendLambData(double activity, int light, int identifier) {
  char msgBuf[11];
  for (int i = 0; i < 11; i++) {
    msgBuf[i] = '\0';
  }
  sprintf(msgBuf, "%02X%04X%04X", (int)(activity * 256), light, identifier);
  lora.send(msgBuf);
  delay(MSG_DELAY);
}

