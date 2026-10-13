#!/usr/bin/env python3

import sounddevice as sd
import numpy as np

print(sd.query_devices());

SAMPLE_RATE = 48000;
DEVICE_ID = 2 #Blackhole 2ch
BIT_LENGTH = 0.1

FREQ_0 = 1800
FREQ_1 = 2200

THRESHOLD = 0.005


WINDOW_SIZE = int(SAMPLE_RATE * BIT_LENGTH)
audio_buffer = np.array([], dtype=np.float32)

def goertzel(samples, frequency, samplerate):
    n = len(samples)
    k = round(n * frequency / samplerate)
    omega = 2 * np.pi * k / n
    coeff = 2 * np.cos(omega)

    s1 = 0.0
    s2 = 0.0

    for sample in samples:
        s0 = sample + coeff * s1 - s2
        s2 = s1
        s1 = s0

    return s1 * s1 + s2 * s2 - coeff * s1 * s2


def audio_callback(indata, frames, time, status):
    global audio_buffer

    if status:
        print("Audiostatus:", status)

    audio_buffer = np.concatenate((audio_buffer, indata[:, 0]))

    while len(audio_buffer) >= WINDOW_SIZE:
        window = audio_buffer[:WINDOW_SIZE]
        audio_buffer = audio_buffer[WINDOW_SIZE:]

        rms = float(np.sqrt(np.mean(window ** 2)))

        if rms < THRESHOLD :
            continue

        power_0 = goertzel(window, FREQ_0, SAMPLE_RATE)
        power_1 = goertzel(window, FREQ_1, SAMPLE_RATE)

        bit = 0 if power_0 > power_1 else 1

        print(
            f"0: {power_0:.1f} | "
            f"1: {power_1:.1f} | "
            f"Erkannt: {bit}"
        )



print("BlackHole Eingang wird geöffnet")

with sd.InputStream(
        device = DEVICE_ID,
        samplerate = SAMPLE_RATE,
        channels = 1,
        dtype = "float32",
        blocksize=480,
        callback = audio_callback
):
    print("Decoder läuft")
    try:
        while True:
            sd.sleep(1000)
    except KeyboardInterrupt:
        pass
