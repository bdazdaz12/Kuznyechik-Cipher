#ifndef GRASSHOPPER_HMACGENERATOR_H
#define GRASSHOPPER_HMACGENERATOR_H

#include "../streebog/StreebogHash.h"

class HmacGenerator {
public:
    HmacGenerator() {
        opad = ByteArray(64, 0x36);
        ipad = ByteArray(64, 0x5c);
    }

    std::shared_ptr<uint8_t[]> calculateHMAC(const ByteArray &secretKey, FILE *inputFile, std::size_t digestSize);

private:
    ByteArray opad;
    ByteArray ipad;
};

#endif //GRASSHOPPER_HMACGENERATOR_H
