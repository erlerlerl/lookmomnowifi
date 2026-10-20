#pragma once

#include <Arduino.h>
#include "driver/i2s.h"

class AudioTransmitter {

    public:
        explicit AudioTransmitter(i2s_port_t port);
        void playTone(float frequency, int duration);
        void sendBit(int bit);
        void sendBits(const int *bits, int length);

    private:
        static constexpr int TX_BUFFER_SIZE = 256;
        int16_t txBuffer[TX_BUFFER_SIZE];
        float phase = 0.0f;

        static constexpr float AMPLITUDE = 2000.f;
        i2s_port_t port;
};
