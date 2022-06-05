//
// Created by yevsyukof on 05.06.22.
//
#include "HmacGenerator.h"
#include "stdio.h"

#define READ_BUFFER_SIZE 65536

std::shared_ptr<uint8_t[]> HmacGenerator::calculateHMAC(const ByteArray &secretKey, FILE *inputFile,
                                                        std::size_t digestSize) {
    ByteArray opad(64, 0x36);
    ByteArray ipad(64, 0x5c);

    StreebogHash expandKeyHash(512);
    ByteArray K_0(expandKeyHash.calculateHash(secretKey), 512 / 8);

    StreebogHash H(digestSize);


    std::shared_ptr<uint8_t[]> buffer(new uint8_t [READ_BUFFER_SIZE], std::default_delete<uint8_t[]>());
    std::size_t bufferLen;

    H.addDataToCTX(K_0 ^ ipad);
    while ((bufferLen = fread(buffer.get(), (size_t) 1, (size_t) READ_BUFFER_SIZE, inputFile))) {
        H.addDataToCTX(buffer, bufferLen);
    }
    buffer = H.finalize();
    bufferLen = digestSize / 8;

    K_0 ^= opad;
    H.addDataToCTX(K_0);
    H.addDataToCTX(buffer, bufferLen);

    return H.finalize();
}

