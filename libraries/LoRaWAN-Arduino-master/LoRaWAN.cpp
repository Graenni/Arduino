#include <Arduino.h>
#include "LoRaWAN.h"

LoRaWAN::LoRaWAN(Stream &s) {
    _s = &s;
};

void LoRaWAN::join() {
    _s->write("AT+JOIN\r\n");
    delay(5000);
}

void LoRaWAN::send(const char* str) {
    if (strlen(str) <= 12) {
        _s->write("AT+SEND=");
        _s->write(str);
        _s->write("\r\n");
    }
};