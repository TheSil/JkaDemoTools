#include "messagebuffer.h"

DEMO_NAMESPACE_START

MessageBuffer::MessageBuffer() : currentPosition(0), length(0) {
}

void MessageBuffer::clean() {
    currentPosition = length = 0;
}

void MessageBuffer::save(std::ofstream& dest) {
    dest.write((char*)&buffer, length);
}

void MessageBuffer::load(std::ifstream& source, int len) {
    clean();
    source.read((char*)&buffer, len);
    length = len;
}

void MessageBuffer::writeBits(int value, int bitSize) {
    if (bitSize < 0) {
        bitSize = -bitSize;
    }

    value &= (0xffffffff >> (32 - bitSize));
    if (bitSize & 7) {
        int nbits;
        nbits = bitSize & 7;
        for (int i = 0; i < nbits; i++) {
            huffman.putBit(*this, value & 1);
            value >>= 1;
        }
        bitSize -= nbits;
    }
    if (bitSize) {
        for (int i = 0; i < bitSize; i += 8) {
            huffman.offsetTransmit(*this, (value & 0xff));
            value >>= 8;
        }
    }
    length = (currentPosition >> 3) + 1;
}

int MessageBuffer::readBits(int bitSize) {
    int nbits = 0, get = 0, value = 0;
    bool sgn;
    if (bitSize < 0) {
        bitSize = -bitSize;
        sgn = true;
    }
    else {
        sgn = false;
    }

    if (bitSize & 7) {
        nbits = bitSize & 7;
        for (int i = 0; i < nbits; i++) {
            value |= (huffman.getBit(*this) << i);
        }
        bitSize -= nbits;
    }

    if (bitSize) {
        for (int i = 0; i < bitSize; i += 8) {
            get = huffman.offsetReceive(*this);
            value |= (get << (i + nbits));
        }
    }
    if (sgn && (value & (1 << (bitSize - 1))))
        value |= -1 ^ ((1 << bitSize) - 1);

    return value;
}

void MessageBuffer::writeString(const std::string& s, bool big = false) {
    unsigned limit = big ? BIG_INFO_STRING : MAX_STRING_CHARS;

    if (s.size() >= limit) {
        writeBits(0, SIZE_8BITS); //empty string
        return;
    }

    //actual writing
    for (unsigned i = 0; i < s.size(); ++i)
        writeBits((byte)s[i], SIZE_8BITS);
    writeBits(0, SIZE_8BITS); // ending sign
}

std::string MessageBuffer::readString(bool big = false) {
    std::string str;
    unsigned    limit = big ? BIG_INFO_STRING : MAX_STRING_CHARS;

    int c;
    do {
        c = readBits(SIZE_8BITS);
        if (c == 0)
            break;

        str += c;
    } while (str.size() < limit - 1);

    return str;
}

void MessageBuffer::initHuffman() {
    MessageBuffer::huffman.init();
}

DEMO_NAMESPACE_END