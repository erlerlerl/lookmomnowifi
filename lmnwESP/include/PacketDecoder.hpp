#pragma once
#include "Protocol.hpp"

class PacketDecoder {
    public:
        static int findPacketHeader(const int *bits, int bitCount);
        static int bitsToInt(const int *bits, int start, int count);
        static int calculateChecksum(const int *bits, int start, int length);

};
