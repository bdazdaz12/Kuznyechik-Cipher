#ifndef GRASSHOPPER_STREEBOGHASH_H
#define GRASSHOPPER_STREEBOGHASH_H

#include "../util/ByteArray.h"
#include <string>

#include "streebog-core.h"
#include "../util/ByteFlow.h"

class StreebogHash {
public:
    explicit StreebogHash(int digestSize);

    ~StreebogHash();

    /** @details Возвращает память, которую нужно чистить */
    std::shared_ptr<uint8_t[]> calculateHash(unsigned char *str) const;

    std::shared_ptr<uint8_t[]> calculateHash(FILE *file) const;

    std::shared_ptr<uint8_t[]> calculateHash(const ByteArray &byteArray) const;

    std::string convertToHex(const std::shared_ptr<uint8_t[]> &ptr) const;

    int getDigestSize() const {
        return digestSize;
    }
    void addDataToCTX(const std::shared_ptr<uint8_t[]> &byteArray, std::size_t arraySize);

    void addDataToCTX(const ByteArray &byteArray);

    std::shared_ptr<uint8_t[]> finalize();
private:
    StreebogContext *CTX;
    int digestSize;
};


#endif //GRASSHOPPER_STREEBOGHASH_H
