#pragma once

#include <vector>
#include "util/Block64Array.h"
#include "CipherConstants.h"
#include "KuznyechickLUT.h"

/*
 * @warning
 * Двоичные вектора, с которыми работаем в алгоритме,
 * записываются от старшего байта к младшему
 * т.е. Block64Array = (a15, ..., a0)
 * */
class Kuznyechik {
public:
    explicit Kuznyechik(const Block64Array &secret_key);

    void encrypt_block(const Block64Array &input_block, Block64Array &output_block) const;

//    void encrypt_file(const std::string &in_file_name, const std::string &out_file_name) const;

private:
    std::vector<Block64Array> round_keys;

    const static std::vector<Block64Array> round_keys_constants;
};
