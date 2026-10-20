#pragma once


constexpr int SAMPLE_RATE = 48000;
constexpr int BITLENGTH = 50;

constexpr float FREQ_0 = 1800.f;
constexpr float FREQ_1 = 2200.f;

constexpr int PREAMBLE_LENGTH = 16;
const int PREAMBLE[PREAMBLE_LENGTH] = {
1,0,1,0,1,0,1,0,
1,0,1,0,1,0,1,0
};

constexpr int SYNC_LENGTH = 8;
const int SYNC[SYNC_LENGTH] = {1,1,0,1,0,0,1,0};
