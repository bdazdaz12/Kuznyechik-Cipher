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


    /**
     * @arg secretKey - это K из описания HMAC
    */
    ByteArray calculateHMAC(const ByteArray &secretKey, const std::string &text);

    ByteArray calculateHMAC(const ByteArray &secretKey, const ByteFlow &byteFlow);

private:
    void addDataToCTX(const ByteArray &byteArray);

    StreebogContext *CTX;
    int digestSize;
};


#endif //GRASSHOPPER_STREEBOGHASH_H
