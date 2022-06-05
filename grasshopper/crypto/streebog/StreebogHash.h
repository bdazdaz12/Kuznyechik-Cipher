#ifndef GRASSHOPPER_STREEBOGHASH_H
#define GRASSHOPPER_STREEBOGHASH_H

//#include "streebog-enter.h"
#include "../util/ByteArray.h"
#include <string>

class StreebogHash {
public:
    explicit StreebogHash(int digestSize) {
        this->digestSize = digestSize;
    }

    std::uint8_t *calculateHash(unsigned char *str) const;

    std::uint8_t *calculateHash(const ByteArray &in) const;

//    void calculateHash(std::string);

    std::string convertToHex(std::uint8_t *digestSource) const;
private:
    int digestSize;
};


#endif //GRASSHOPPER_STREEBOGHASH_H
