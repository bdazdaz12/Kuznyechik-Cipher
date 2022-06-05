
/**
 * @brief  GOST 34.11-2012 hash function with 512/256 bits digest.
 */

#include "StreebogHash.h"
#include <err.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sysexits.h>
#include <unistd.h>

#define READ_BUFFER_SIZE 65536

#define TEST_BLOCK_LEN 8192


#define DEFAULT_DIGEST_SIZE 512
#define ALGNAME "GOST R 34.11-2012"

//static StreebogContext *CTX;

//uint8_t digest[64];
//unsigned char hexdigest[129];
//unsigned int digest_size = DEFAULT_DIGEST_SIZE;

const union uint512_u GOSTTestInput = {
#ifndef __GOST3411_BIG_ENDIAN__
        {
                0x3736353433323130ULL,
                0x3534333231303938ULL,
                0x3332313039383736ULL,
                0x3130393837363534ULL,
                0x3938373635343332ULL,
                0x3736353433323130ULL,
                0x3534333231303938ULL,
                0x0032313039383736ULL
        }
#else
        {
            0x3031323334353637ULL,
            0x3839303132333435ULL,
            0x3637383930313233ULL,
            0x3435363738393031ULL,
            0x3233343536373839ULL,
            0x3031323334353637ULL,
            0x3839303132333435ULL,
            0x3637383930313200ULL
        }
#endif
};

void CleanupCTX(StreebogContext *CTX) {
    memset(CTX, 0x00, sizeof(StreebogContext));
}

void InitCTX(StreebogContext *CTX, const unsigned int digest_size) {
    unsigned int i;

    memset(CTX, 0x00, sizeof(StreebogContext));
    CTX->digest_size = digest_size;

    for (i = 0; i < 8; i++) {
        if (digest_size == 256) {
            CTX->h.QWORD[i] = 0x0101010101010101ULL;
        } else {
            CTX->h.QWORD[i] = 0x00ULL;
        }
    }
}

void UpdateCTX(StreebogContext *CTX, const unsigned char *data, size_t len) {
    size_t chunksize;

    if (CTX->bufsize) {
        chunksize = 64 - CTX->bufsize;
        if (chunksize > len)
            chunksize = len;

        memcpy(&CTX->buffer[CTX->bufsize], data, chunksize);

        CTX->bufsize += chunksize;
        len -= chunksize;
        data += chunksize;

        if (CTX->bufsize == 64) {
            stage2(CTX, CTX->buffer);

            CTX->bufsize = 0;
        }
    }

    while (len > 63) {
        stage2(CTX, data);

        data += 64;
        len -= 64;
    }

    if (len) {
        memcpy(&CTX->buffer, data, len);
        CTX->bufsize = len;
    }
}

void FinalCTX(StreebogContext *CTX, unsigned char *digest) {
    stage3(CTX);

    CTX->bufsize = 0;

    if (CTX->digest_size == 256) {
        memcpy(digest, &(CTX->hash.QWORD[4]), 32);
    } else {
        memcpy(digest, &(CTX->hash.QWORD[0]), 64);
    }
}

static void *memalloc(const size_t size) {
    void *p;

    /* Ensure p is on a 64-bit boundary. */
    if (posix_memalign(&p, (size_t) 64, size)) {
        err(EX_OSERR, nullptr);
    }
    return p;
}

static void reverse_order(unsigned char *in, size_t len) {
    unsigned char c;
    unsigned int i, j;

    for (i = 0, j = (unsigned int) len - 1; i < j; i++, j--) {
        c = in[i];
        in[i] = in[j];
        in[j] = c;
    }
}

static void convert_to_hex(unsigned char *in, unsigned char *out, size_t len,
                           const unsigned int eflag) {
    unsigned int i;
    char ch[3];

    if (len > 64) len = 64;

    memset(out, 0, 129);

    /* eflag is set when little-endian output requested */
    if (eflag) reverse_order(in, len);

    for (i = 0; i < len; i++) {
        sprintf(ch, "%02x", (unsigned char) in[i]);
        memcpy(&out[i * 2], ch, 2);
    }
}

/**
 * @arg digestSize - размер в битах считаемого хэша
*/
static uint8_t *calculateStringHash(StreebogContext *CTX, unsigned char *const string, unsigned int digestSize) {
    uint8_t digest[64];

    unsigned char *buf __attribute__((aligned(16)));
    size_t size;

    InitCTX(CTX, digestSize);

    size = strnlen((const char *) string, (size_t) 4096);
    buf = (unsigned char *) memalloc(size);
    memcpy(buf, string, size);
    {
        UpdateCTX(CTX, buf, size);
        FinalCTX(CTX, &digest[0]);
        CleanupCTX(CTX);
    }

    auto *calculatedHash = new uint8_t[digestSize / 8];
    memcpy(calculatedHash, digest, digestSize / 8);

    return calculatedHash;
}

static uint8_t *calculateByteArrayHash(StreebogContext *CTX, const ByteArray &byteArray, unsigned int digestSize) {
    uint8_t digest[64];

    InitCTX(CTX, digestSize);
    {
        UpdateCTX(CTX, byteArray.getArrayPtr(), byteArray.size());
        FinalCTX(CTX, &digest[0]);
        CleanupCTX(CTX);
    }

    auto *calculatedHash = new uint8_t[digestSize / 8];
    memcpy(calculatedHash, &digest[0], digestSize / 8);

    return calculatedHash;
}


#ifdef SUPERCOP
#include "crypto_hash.h"

int crypto_hash(unsigned char *out, const unsigned char *in, unsigned long long inlen) {
    CTX = memalloc(sizeof(StreebogContext));

    GOST34112012Init(CTX, 512);
    GOST34112012Update(CTX, in, (size_t) inlen);
    GOST34112012Final(CTX, out);

    return 0;
}
#endif


////////////////////////////////////////////////////////////////////////////////////////////////


std::uint8_t *StreebogHash::calculateHash(unsigned char *str) const {
    return calculateStringHash(CTX, str, digestSize);
}


std::string StreebogHash::convertToHex(std::uint8_t *digest) const {
    unsigned int i;
    char ch[3];
    char hexdigest[129];

    memset(hexdigest, 0, 129);

    for (i = 0; i < digestSize / 8; i++) {
        sprintf(ch, "%02x", (unsigned char) digest[i]);
        memcpy(&hexdigest[i * 2], ch, 2);
    }

    std::string ans(hexdigest);
    return ans;
}

std::uint8_t *StreebogHash::calculateHash(const ByteArray &in) const {
    return calculateByteArrayHash(CTX, in, digestSize);
}

StreebogHash::StreebogHash(int digestSize) {
    if(digestSize != 256 && digestSize != 512) {
        std::cerr << "Streebog: задан недопустимый размер дайджеста" << std::endl;
        exit(1);
    }

    this->CTX = (GOST34112012Context *) memalloc(sizeof(GOST34112012Context));
    this->digestSize = digestSize;
}

StreebogHash::~StreebogHash() {
    if (CTX != nullptr) {
        CleanupCTX(CTX);
    }
}

//std::uint8_t *StreebogHash::hashStr(const unsigned char *string) {
//    auto *digest = new uint8_t[64];
//    char hexdigest[129];
//
//    unsigned char *buf __attribute__((aligned(16)));
//    size_t size;
//
//    InitCTX(CTX, digestSize);
//
//    size = strnlen((const char *) string, (size_t) 4096);
//    buf = (unsigned char *) memalloc(size);
//    memcpy(buf, string, size);
//
//    {
//        UpdateCTX(CTX, buf, size);
//        FinalCTX(CTX, &digest[0]);
//        CleanupCTX(CTX);
//    }
//
//
//    if (digestSize == 256) {
//        convert_to_hex(digest, reinterpret_cast<unsigned char *>(hexdigest), 32, 0);
//    } else {
//        convert_to_hex(digest, reinterpret_cast<unsigned char *>(hexdigest), 64, 0);
//    }
//
//
//    std::string str(hexdigest);
//    printf("%s\n", hexdigest);
//
//    std::cout << std::endl << str;
//
//    return nullptr;
//}
