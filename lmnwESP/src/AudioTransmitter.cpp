#include "AudioTransmitter.hpp"
#include "Protocol.hpp"
#include "driver/i2s.h"
#include <math.h>

AudioTransmitter::AudioTransmitter(i2s_port_t port) : port(port) {}

void AudioTransmitter::playTone(float frequency, int duration) { //duration in ms
    // const float amplitude = 10000.f;
    static float phase = 0.0f;

    const float phaseIncrement = 2.0f * PI * frequency / SAMPLE_RATE;

    const int totalSamples = SAMPLE_RATE * duration / 1000;

    int samplesWritten = 0;

    while (samplesWritten < totalSamples) {

        for (int i = 0; i < TX_BUFFER_SIZE; i++) {
            txBuffer[i] = (int16_t)(AMPLITUDE * sinf(phase));
            phase += phaseIncrement;

            if (phase >= 2 * PI) {
                phase-=2*PI;
            }
        }

        size_t bytesWritten = 0;

        i2s_write(port, txBuffer, sizeof(txBuffer), &bytesWritten,
                    portMAX_DELAY
        );

        samplesWritten += bytesWritten / sizeof(int16_t);
    }

}

void AudioTransmitter::sendBit(int bit) {
   const float frequency = bit == 0 ? FREQ_0 : FREQ_1;
   playTone(frequency, BITLENGTH);
}

void AudioTransmitter::sendBits(const int *bits, int length) {
    for (int i = 0; i < length; i++) {
        sendBit(bits[i]);
  }
}
