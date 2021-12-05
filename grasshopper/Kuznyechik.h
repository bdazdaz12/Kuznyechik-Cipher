#ifndef __KUZNYECHIK__
#define __KUZNYECHIK__

#include <vector>
#include "ByteArray.h"
#include "KuznyechikConstants.h"

/*
 * @warning
 * Двоичные вектора, с которыми работаем в алгоритме,
 * записываются от старшего байта к младшему
 * т.е. ByteArray = (a15, ..., a0)
 * */
class Kuznyechik {
public:
    explicit Kuznyechik(const ByteArray &secret_key);

    void encrypt_block(const ByteArray &input_block, ByteArray &output_block) const;

    void decrypt_block(const ByteArray &input_block, ByteArray &output_block) const;

    void encrypt_file(const std::string &in_file_name, const std::string &out_file_name) const;

    void decrypt_file(const std::string &in_file_name, const std::string &out_file_name) const;


private:
    std::vector<ByteArray> round_keys;
};

#endif
