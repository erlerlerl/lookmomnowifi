#!/usr/bin/env python3

import sounddevice as sd
import numpy as np

print(sd.query_devices());

SAMPLE_RATE = 48000;
DEVICE_ID = 2 #Blackhole 2ch

# DEVICE_ID = 3 #MacBook Mikrofon
BIT_LENGTH = 0.005
PREAMBLE = [1,0] * 8
SYNC = [1,1,0,1,0,0,1,0]

FREQ_0 = 1800
FREQ_1 = 2200

THRESHOLD = 0.005


WINDOW_SIZE = int(SAMPLE_RATE * BIT_LENGTH)
audio_buffer = np.array([], dtype=np.float32)
received_bits = []
reported_packets = set()

def bits_to_int(bits):
    value = 0
    for bit in bits:
        value = (value << 1) | bit
    return value

def bits_to_ascii(bits):
    chars = []

    for start in range(0, len(bits) -7, 8):
        byte = bits_to_int(bits[start:start+8])
        chars.append(chr(byte))

    return "".join(chars)

def checksum_bits(payload):
    checksum = 0

    for start in range(0, len(payload), 8):
        group = payload[start:start+8]
        group = group + [0] * (8 -len(group))
        checksum ^= bits_to_int(group)

    return [
        (checksum >> shift) & 1
        for shift in range(7,-1,-1)
    ]

def decode_packets(bits):

    packets = []
    i = 0

    while i <= len(bits) - 32:
        if bits[i:i+16] != PREAMBLE:
            i += 1
            continue

        sync_start = i + 16

        if bits[sync_start:sync_start + 8] != SYNC:
            i += 1
            continue

        length_start = sync_start + 8

        length = bits_to_int(bits[length_start:length_start+8])

        payload_start = length_start+8

        payload_end = payload_start + length

        checksum_end = payload_end + 8

        if checksum_end > len(bits):
            break

        payload = bits[payload_start:payload_end]
        received_checksum = bits[payload_end:checksum_end]
        expected_checksum = checksum_bits(payload)

        if received_checksum == expected_checksum:
            packet = tuple(payload)

            if packet not in reported_packets:
                reported_packets.add(packet)
                text = bits_to_ascii(payload)
                print("Received Packet:", text)

            packets.append(payload)
        else:
            print("Packet ignored: Wrong Checksum")

        i = checksum_end

    return packets


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
    global audio_buffer, received_bits

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

        received_bits.append(bit)

        if len(received_bits) > 4096:
            del received_bits[:-2048]

        decode_packets(received_bits)

        # print(
        #     f"0: {power_0:.1f} | "
        #     f"1: {power_1:.1f} | "
        #     f"Erkannt: {bit}"
        # )



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
