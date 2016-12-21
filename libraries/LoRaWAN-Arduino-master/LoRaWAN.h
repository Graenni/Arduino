#ifndef LORAWAN_H
#define LORAWAN_H

#include <Arduino.h>
#include <Stream.h>

class LoRaWAN
{
public:
    LoRaWAN(Stream &s);
    void join();
    void send(const char* str);
private:
    Stream* _s;
};

#endif