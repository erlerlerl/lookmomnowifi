#include <Arduino.h>
#include "driver/i2s.h"

constexpr int I2S_BCLK = 26;
constexpr int I2S_WS = 25;
constexpr int I2S_DIN = 33;
constexpr int I2S_DOUT = 22;

constexpr i2s_port_t I2S_PORT = I2S_NUM_0;
constexpr int FRAMES = 256;
int32_t i2sBuffer[FRAMES];
constexpr int SAMPLE_RATE = 48000;
constexpr int BITLENGTH = 50;
constexpr int WINDOW_SIZE = SAMPLE_RATE * BITLENGTH / 1000;
float windowBuffer[WINDOW_SIZE];
int windowIndex = 0;

constexpr float THRESHOLD = 0.01f;
constexpr float POWER_THRESHOLD = 100.0f;

constexpr float FREQ_0 = 1800.f;
constexpr float FREQ_1 = 2200.f;

constexpr int MAX_RECEIVED_BITS = 4096;

int receivedBits[MAX_RECEIVED_BITS];
int receivedBitCount = 0;


void setupI2S() {
    i2s_config_t config = {};
    config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX);

    config.sample_rate = SAMPLE_RATE;
    config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
    config.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;
    config.communication_format = I2S_COMM_FORMAT_I2S;
    config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    config.dma_buf_count = 8;
    config.dma_buf_len = FRAMES;
    config.tx_desc_auto_clear = true;
    config.fixed_mclk = 0;

    esp_err_t err = i2s_driver_install(I2S_PORT, &config, 0, nullptr);
    if (err != ESP_OK) {
        Serial.printf("I2S driver error: %d\n", err);
        while (true)
            delay(1000);
    }

    i2s_pin_config_t pins = {};
    pins.bck_io_num = I2S_BCLK;
    pins.ws_io_num = I2S_WS;
    pins.data_out_num = I2S_DOUT;
    pins.data_in_num = I2S_DIN;
    pins.mck_io_num = I2S_PIN_NO_CHANGE;

    err = i2s_set_pin(I2S_PORT, &pins);
    if (err != ESP_OK) {
        Serial.printf("I2S pin error: %d\n", err);
        while (true)
            delay(1000);
    }

    i2s_zero_dma_buffer(I2S_PORT);
}

float calculateRMS(const float* samples, int count) {
    float sumSquares = 0.0f;

    for(int i = 0; i < count; i++){
        sumSquares += samples[i] * samples[i];
    }

    return sqrtf(sumSquares / count);
}

float goertzel(const float *samples, int count, float frequency) {
    int k = roundf((float)count * frequency / SAMPLE_RATE);
    float omega = 2.0f * PI * k / count;
    float coeff = 2.0f * cosf(omega);

    float s1 = 0.0f;
    float s2 = 0.0f;

    for (int i = 0; i < count; i++) {
        float s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    return s1 * s1 + s2 * s2 - coeff * s1 * s2;
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.printf("Window size: %d\n", WINDOW_SIZE);
    setupI2S();
}

void loop() {
    size_t bytesRead = 0;

    esp_err_t err = i2s_read(I2S_PORT, i2sBuffer, sizeof(i2sBuffer),
                             &bytesRead, portMAX_DELAY);


    if (err != ESP_OK || bytesRead == 0) {
        return;
    }


    size_t samplesRead = bytesRead / sizeof(int32_t);

    for(size_t i = 0; i < samplesRead; i++){
        float sample = (float) i2sBuffer[i] / 2147483648.0f;

        windowBuffer[windowIndex] = sample;

        windowIndex++;

        if(windowIndex >= WINDOW_SIZE){
            float rms = calculateRMS(windowBuffer, WINDOW_SIZE);
            if (rms >= THRESHOLD) {

                float power0 = goertzel(windowBuffer, WINDOW_SIZE, FREQ_0);
                float power1 = goertzel(windowBuffer, WINDOW_SIZE, FREQ_1);

                float maxPower = fmaxf(power0, power1);

                if (maxPower > POWER_THRESHOLD){
                    int bit = (power0 > power1) ? 0 : 1;
                    if (receivedBitCount < MAX_RECEIVED_BITS) {
                        receivedBits[receivedBitCount] = bit;
                        receivedBitCount++;
                    }
                    // Serial.printf("0: %..2f | 1: %.2f | Bit: %d\n", power0, power1, bit);
                    Serial.printf("Bit: %d | Gesamt: %d\n", bit, receivedBitCount);
                  }
            }
            windowIndex = 0;
        }
    }

}
