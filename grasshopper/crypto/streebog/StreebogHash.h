#ifndef GRASSHOPPER_STREEBOGHASH_H
#define GRASSHOPPER_STREEBOGHASH_H

#include "../util/ByteArray.h"
#include <string>

#include "streebog-core.h"

class StreebogHash {
public:
    explicit StreebogHash(int digestSize);

    ~StreebogHash();

    /** @details Возвращает память, которую нужно чистить */
    std::uint8_t *calculateHash(unsigned char *str) const;

    std::uint8_t *calculateHash(const ByteArray &in) const;

//    void calculateHash(std::string);

    int getDigestSize() const {
        return digestSize;
    }

    std::string convertToHex(std::uint8_t *digest) const;
private:
    StreebogContext *CTX;
    int digestSize;
};


#endif //GRASSHOPPER_STREEBOGHASH_H
