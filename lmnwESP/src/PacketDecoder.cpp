#include "PacketDecoder.hpp"


int PacketDecoder::findPacketHeader(const int *bits, int bitCount) {
    constexpr int HEADER_LENGTH = PREAMBLE_LENGTH + SYNC_LENGTH;

    if (bitCount < HEADER_LENGTH) {
        return -1;
    }

    for (int start = 0; start <= bitCount - HEADER_LENGTH; start++) {
        bool match = true;

        //check preamble
        for (int j = 0; j < PREAMBLE_LENGTH; j++) {
            if (bits[start + j] != PREAMBLE[j]) {
                match = false;
                break;
          }
        }

        if (!match) {
            continue;
        }

        for (int j = 0; j < SYNC_LENGTH; j++) {
            if (bits[start + PREAMBLE_LENGTH + j] != SYNC[j]) {
                match = false;
                break;
          }
        }

        if (match) {
            return start;
        }

    }

    return -1;
}

int PacketDecoder::bitsToInt(const int *bits, int start, int count) {
    int value = 0;

    for (int i = 0; i < count; i++) {
        value = (value << 1) | bits[start+i];
    }

    return value;
}


int PacketDecoder::calculateChecksum(const int *bits, int start, int length) {
    int checksum = 0;

    for (int offset = 0; offset < length; offset += 8) {
        int groupLength = length - offset;

        if (groupLength > 8) {
            groupLength = 8;
        }

        int value = bitsToInt(bits, start + offset, groupLength);

        if (groupLength < 8) {
            value <<= (8-groupLength);
        }

        checksum ^= value;
    }

    return checksum;
}
