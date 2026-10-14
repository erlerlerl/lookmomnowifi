#include <Arduino.h>
#include "driver/i2s.h"

constexpr int I2S_BCLK = 26;
constexpr int I2S_WS = 25;
constexpr int I2S_DIN = 33;
constexpr int I2S_DOUT = 22;

constexpr i2s_port_t I2S_PORT = I2S_NUM_0;
constexpr int SAMPLE_RATE = 16000;
constexpr size_t FRAMES = 256;

int32_t inputBuffer[FRAMES];
int32_t outputBuffer[FRAMES];

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

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("ESP32 I2S microphone -> speaker test");
    setupI2S();
    Serial.println("Ready! Speak near the mic.");
}

void loop() {
    size_t bytesRead = 0;
    size_t bytesWritten = 0;

    esp_err_t err = i2s_read(I2S_PORT, inputBuffer, sizeof(inputBuffer),
                             &bytesRead, portMAX_DELAY);

    static unsigned long lastPrint = 0;

    if(millis() - lastPrint >= 1000){
        lastPrint = millis();

        Serial.printf("Read: %u bytes, error: %d\n", (unsigned) bytesRead, (int) err);
    }

    if (err != ESP_OK || bytesRead == 0) {
        return;
    }


    size_t samples = bytesRead / sizeof(int32_t);

    for (size_t i = 0; i < samples; i++){
      int32_t sample16 = inputBuffer[i] >> 16; // reduce 32 bit value to 16 bit
      outputBuffer[i] = sample16 << 16;
    }

    i2s_write(I2S_PORT, outputBuffer, samples*sizeof(int32_t), &bytesWritten, portMAX_DELAY);
}
