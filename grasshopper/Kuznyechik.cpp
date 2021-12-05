#include <stdexcept>
#include <vector>
#include <map>
#include <iostream>
#include <cstring>

#include "Kuznyechik.h"

void applyLSX(const ByteArray &round_key, ByteArray &output_block);

uint16_t multiply(uint16_t a, uint16_t b) {
    uint16_t result = 0;
    uint16_t modulus = linear_transform_modulus << 7;
    for (uint16_t detecter = 0x1; detecter != 0x100; detecter <<= 1, a <<= 1) {
        if (b & detecter) {
            result ^= a;
        }
    }
    for (uint16_t detecter = 0x8000; detecter != 0x80; detecter >>= 1, modulus >>= 1) {
        if (result & detecter) {
            result ^= modulus;
        }
    }
    return result;
}

void nonlinear_transform(ByteArray &output_block) {
    for (int i = 0; i < output_block.size(); ++i) {
        output_block[i] = S[output_block[i]];
    }
}

void inverse_nonlinear_transform(ByteArray &output_block) {
    for (int i = 0; i < output_block.size(); ++i) {
        output_block[i] = inverse_S[output_block[i]];
    }
}

uint8_t round_linear_transform_core(const ByteArray &output_block) {
    uint16_t result = 0;
    for (int i = 0; i < ROUND_KEY_LENGTH; i++) {
        result ^= multiply(output_block[i], linear_transform_coefficients[i]);
    }
    return result;
}

void round_linear_transform(ByteArray &output_block) {
    uint8_t sponge = round_linear_transform_core(output_block);
    for (int i = ROUND_KEY_LENGTH - 1; i > 0; --i) {
        output_block[i] = output_block[i - 1];
    }
    output_block[0] = sponge;
}

void inverse_round_linear_transform(ByteArray &output_block) {
    uint8_t tmp = output_block[0];
    for (int i = 0; i < ROUND_KEY_LENGTH - 1; ++i) {
        output_block[i] = output_block[i + 1];
    }
    output_block[15] = tmp;
    output_block[15] = round_linear_transform_core(output_block);
}

void linear_transform(ByteArray &output_block) {
    for (int i = 0; i < ROUND_KEY_LENGTH; ++i) {
        round_linear_transform(output_block);
    }
}

void inverse_linear_transform(ByteArray &output_block) {
    for (int i = 0; i < ROUND_KEY_LENGTH; ++i) {
        inverse_round_linear_transform(output_block);
    }
}

void init_round_keys_constants() { // C_i = L(Vec_128(i))
    auto *round_keys_constants_initializer
            = const_cast< std::vector<ByteArray> * >(&round_keys_constants);
    ByteArray v128;
    for (uint8_t i = 1; i <= 32; ++i) {
        v128 = ByteArray(ROUND_KEY_LENGTH, 0);
        v128[ROUND_KEY_LENGTH - 1] = i;
        linear_transform(v128);
        round_keys_constants_initializer->push_back(std::move(v128));
    }
}

void calc_round_keys(std::vector<ByteArray> &round_keys, const ByteArray &secret_key) {
    round_keys[0].replace_array(secret_key.get_byte_arr_ptr(),
                                ROUND_KEY_LENGTH);
    round_keys[1].replace_array(secret_key.get_byte_arr_ptr() + ROUND_KEY_LENGTH,
                                ROUND_KEY_LENGTH);

    for (int i = 0; i < 4; ++i) {
        ByteArray a1 = round_keys[2 * i].create_copy();
        ByteArray a2 = round_keys[2 * i + 1].create_copy();

        for (int j = 0; j < 8; ++j) { // F_transform
            ByteArray a1_copy = a1.create_copy();

            applyLSX(round_keys_constants[8 * i + j], a1);
            a1 ^= a2;

            a2 = a1_copy;
        }

        round_keys[2 * i + 2] = a1;
        round_keys[2 * i + 3] = a2;
    }
}

Kuznyechik::Kuznyechik(const ByteArray &secret_key) : round_keys(NUMBER_OF_ROUNDS) {
    if (secret_key.size() != 32) {
        throw std::invalid_argument("Kuznyechik: The secret_key must be 32 bytes long");
    }

    init_round_keys_constants();
    calc_round_keys(this->round_keys, secret_key);
}

void applyLSX(const ByteArray &round_key, ByteArray &output_block) {
    output_block ^= round_key; // X
    nonlinear_transform(output_block); // S
    linear_transform(output_block); // L
}

void Kuznyechik::encrypt_block(const ByteArray &input_block, ByteArray &output_block) const { // TODO
    if (input_block.size() != ROUND_KEY_LENGTH) {
        throw std::invalid_argument("Kuznyechik: The block must be 16 bytes length");
    }
    if (output_block != input_block) {
        output_block = input_block;
    }

    for (int i = 0; i < NUMBER_OF_ROUNDS - 1; ++i) {
        applyLSX(round_keys[i], output_block);
    }
    output_block ^= round_keys[NUMBER_OF_ROUNDS - 1];
}

void Kuznyechik::decrypt_block(const ByteArray &input_block, ByteArray &output_block) const {
    if (input_block.size() != ROUND_KEY_LENGTH) {
        throw std::invalid_argument("Kuznyechik: The block must be 16 bytes length");
    }
    if (output_block != input_block) {
        output_block = input_block;
    }

    output_block ^= round_keys[NUMBER_OF_ROUNDS - 1];
    for (int i = NUMBER_OF_ROUNDS - 2; i >= 0; --i) {
        inverse_linear_transform(output_block);
        inverse_nonlinear_transform(output_block);
        output_block ^= round_keys[i];
    }
}

void Kuznyechik::encrypt_file(const std::string &in_file_name,
                              const std::string &out_file_name) const {
    // TODO
}

void Kuznyechik::decrypt_file(const std::string &in_file_name,
                              const std::string &out_file_name) const {
    //TODO
}
