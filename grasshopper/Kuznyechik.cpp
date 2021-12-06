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

void nonlinear_transform(ByteArray &output_block) { /// norm
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

void round_linear_transform(ByteArray &output_block) { // TODO оптимизировать
    uint8_t sponge = round_linear_transform_core(output_block);
    for (int i = INPUT_BLOCK_LENGTH - 1; i > 0; --i) {
        output_block[i] = output_block[i - 1];
    }
    output_block[0] = sponge;
}

void inverse_round_linear_transform(ByteArray &output_block) {
    uint8_t tmp = output_block[0];
    for (int i = 0; i < INPUT_BLOCK_LENGTH - 1; ++i) {
        output_block[i] = output_block[i + 1];
    }
    output_block[15] = tmp;
    output_block[15] = round_linear_transform_core(output_block);
}

void linear_transform(ByteArray &output_block) {
    for (int i = 0; i < INPUT_BLOCK_LENGTH; ++i) {
        round_linear_transform(output_block);
    }
}

void inverse_linear_transform(ByteArray &output_block) {
    for (int i = 0; i < INPUT_BLOCK_LENGTH; ++i) {
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
                              const std::string &out_file_name) const {  /// IN MEMORY
    FILE *in_file_ptr = std::fopen(in_file_name.c_str(), "rb");
    FILE *out_file_ptr = std::fopen(out_file_name.c_str(), "wb");

    constexpr int buffer_size = 1000 * 32768;
    uint8_t read_buffer[buffer_size];

    std::size_t cnt_read_bytes;
    ByteArray cur_in_block;
    while ((cnt_read_bytes = fread(read_buffer, sizeof(uint8_t), buffer_size, in_file_ptr))) {
        for (int i = 0; i < cnt_read_bytes / INPUT_BLOCK_LENGTH; ++i) {
            cur_in_block.absorb(read_buffer + i * INPUT_BLOCK_LENGTH,
                                INPUT_BLOCK_LENGTH);
            encrypt_block(cur_in_block, cur_in_block);
        }
        fwrite(read_buffer, sizeof(uint8_t), cnt_read_bytes, out_file_ptr);
    }
    cur_in_block.absorb(nullptr, 0);

    fclose(in_file_ptr);
    fclose(out_file_ptr);
}

/// IN MEMORY
//void Kuznyechik::encrypt_file(const std::string &in_file_name,
//                              const std::string &out_file_name) const {
//    FILE *in_file_ptr = std::fopen(in_file_name.c_str(), "rb");
//    FILE *out_file_ptr = std::fopen(out_file_name.c_str(), "wb");
//
//    constexpr int buffer_size = 65536;
//    auto *read_buffer = new uint8_t[buffer_size];
//    if (buffer_size == fread(read_buffer, sizeof(uint8_t), buffer_size, in_file_ptr)) {
//        ByteArray cur_in_block;
//        for (long long i = 0; i < buffer_size / INPUT_BLOCK_LENGTH; ++i) {
//            cur_in_block.absorb(read_buffer + i * INPUT_BLOCK_LENGTH,
//                                INPUT_BLOCK_LENGTH);
//            encrypt_block(cur_in_block, cur_in_block);
//        }
//        cur_in_block.absorb(nullptr, 0);
//    }
//
//    delete[] read_buffer;
//
//    fclose(in_file_ptr);
//    fclose(out_file_ptr);
//}

void Kuznyechik::decrypt_file(const std::string &in_file_name,
                              const std::string &out_file_name) const {
    FILE *in_file_ptr = std::fopen(in_file_name.c_str(), "rb");
    FILE *out_file_ptr = std::fopen(out_file_name.c_str(), "wb");

    constexpr int buffer_size = 32768;
    uint8_t read_buffer[buffer_size];

    std::size_t cnt_read_bytes;
    ByteArray cur_in_block;
    while ((cnt_read_bytes = fread(read_buffer, sizeof(uint8_t), buffer_size, in_file_ptr))) {
        for (int i = 0; i < cnt_read_bytes / INPUT_BLOCK_LENGTH; ++i) {
            cur_in_block.absorb(read_buffer + i * INPUT_BLOCK_LENGTH,
                                INPUT_BLOCK_LENGTH);
            decrypt_block(cur_in_block, cur_in_block);
        }
        fwrite(read_buffer, sizeof(uint8_t), cnt_read_bytes, out_file_ptr);
    }
    cur_in_block.absorb(nullptr, 0);

    fclose(in_file_ptr);
    fclose(out_file_ptr);
}

//static inline void LS(uint64_t x1, uint64_t x2, uint64_t &t1, uint64_t &t2) { // TODO
//    t1 = LS_table[0][(unsigned char) (x1)][0] ^
//         LS_table[1][(unsigned char) (x1 >> 8)][0] ^
//         LS_table[2][(unsigned char) (x1 >> 16)][0] ^
//         LS_table[3][(unsigned char) (x1 >> 24)][0] ^
//         LS_table[4][(unsigned char) (x1 >> 32)][0] ^
//         LS_table[5][(unsigned char) (x1 >> 40)][0] ^
//         LS_table[6][(unsigned char) (x1 >> 48)][0] ^
//         LS_table[7][(unsigned char) (x1 >> 56)][0] ^
//         LS_table[8][(unsigned char) (x2)][0] ^
//         LS_table[9][(unsigned char) (x2 >> 8)][0] ^
//         LS_table[10][(unsigned char) (x2 >> 16)][0] ^
//         LS_table[11][(unsigned char) (x2 >> 24)][0] ^
//         LS_table[12][(unsigned char) (x2 >> 32)][0] ^
//         LS_table[13][(unsigned char) (x2 >> 40)][0] ^
//         LS_table[14][(unsigned char) (x2 >> 48)][0] ^
//         LS_table[15][(unsigned char) (x2 >> 56)][0];
//    t2 = T[0][(unsigned char) (x1)][1] ^
//         T[1][(unsigned char) (x1 >> 8)][1] ^
//         T[2][(unsigned char) (x1 >> 16)][1] ^
//         T[3][(unsigned char) (x1 >> 24)][1] ^
//         T[4][(unsigned char) (x1 >> 32)][1] ^
//         T[5][(unsigned char) (x1 >> 40)][1] ^
//         T[6][(unsigned char) (x1 >> 48)][1] ^
//         T[7][(unsigned char) (x1 >> 56)][1] ^
//         T[8][(unsigned char) (x2)][1] ^
//         T[9][(unsigned char) (x2 >> 8)][1] ^
//         T[10][(unsigned char) (x2 >> 16)][1] ^
//         T[11][(unsigned char) (x2 >> 24)][1] ^
//         T[12][(unsigned char) (x2 >> 32)][1] ^
//         T[13][(unsigned char) (x2 >> 40)][1] ^
//         T[14][(unsigned char) (x2 >> 48)][1] ^
//         T[15][(unsigned char) (x2 >> 56)][1];
//}

void Kuznyechik::fast_encrypt_block(const ByteArray &input_block, ByteArray &output_block) const {
    static uint64_t i[2];
    static uint64_t o[2];

    auto *input_block_arr = input_block.kuzn_cast(); // костыль, работает только если in = out_block

    i[0] = input_block_arr[0] ^ round_keys[0].kuzn_cast()[0];
    i[1] = input_block_arr[1] ^ round_keys[0].kuzn_cast()[1];

    o[0] = LS_table[0][(i[1] >> (7 * 8)) & 0xFF][0] ^
           LS_table[1][(i[1] >> (6 * 8)) & 0xFF][0] ^
           LS_table[2][(i[1] >> (5 * 8)) & 0xFF][0] ^
           LS_table[3][(i[1] >> (4 * 8)) & 0xFF][0] ^
           LS_table[4][(i[1] >> (3 * 8)) & 0xFF][0] ^
           LS_table[5][(i[1] >> (2 * 8)) & 0xFF][0] ^
           LS_table[6][(i[1] >> (1 * 8)) & 0xFF][0] ^
           LS_table[7][(i[1] >> (0 * 8)) & 0xFF][0] ^
           LS_table[8][(i[0] >> (7 * 8)) & 0xFF][0] ^
           LS_table[9][(i[0] >> (6 * 8)) & 0xFF][0] ^
           LS_table[10][(i[0] >> (5 * 8)) & 0xFF][0] ^
           LS_table[11][(i[0] >> (4 * 8)) & 0xFF][0] ^
           LS_table[12][(i[0] >> (3 * 8)) & 0xFF][0] ^
           LS_table[13][(i[0] >> (2 * 8)) & 0xFF][0] ^
           LS_table[14][(i[0] >> (1 * 8)) & 0xFF][0] ^
           LS_table[15][(i[0] >> (0 * 8)) & 0xFF][0] ^ round_keys[1].kuzn_cast()[0];

    o[1] = LS_table[0][(i[1] >> (7 * 8)) & 0xFF][1] ^
           LS_table[1][(i[1] >> (6 * 8)) & 0xFF][1] ^
           LS_table[2][(i[1] >> (5 * 8)) & 0xFF][1] ^
           LS_table[3][(i[1] >> (4 * 8)) & 0xFF][1] ^
           LS_table[4][(i[1] >> (3 * 8)) & 0xFF][1] ^
           LS_table[5][(i[1] >> (2 * 8)) & 0xFF][1] ^
           LS_table[6][(i[1] >> (1 * 8)) & 0xFF][1] ^
           LS_table[7][(i[1] >> (0 * 8)) & 0xFF][1] ^
           LS_table[8][(i[0] >> (7 * 8)) & 0xFF][1] ^
           LS_table[9][(i[0] >> (6 * 8)) & 0xFF][1] ^
           LS_table[10][(i[0] >> (5 * 8)) & 0xFF][1] ^
           LS_table[11][(i[0] >> (4 * 8)) & 0xFF][1] ^
           LS_table[12][(i[0] >> (3 * 8)) & 0xFF][1] ^
           LS_table[13][(i[0] >> (2 * 8)) & 0xFF][1] ^
           LS_table[14][(i[0] >> (1 * 8)) & 0xFF][1] ^
           LS_table[15][(i[0] >> (0 * 8)) & 0xFF][1] ^ round_keys[1].kuzn_cast()[1];

    i[0] = LS_table[0][(o[1] >> (7 * 8)) & 0xFF][0] ^
           LS_table[1][(o[1] >> (6 * 8)) & 0xFF][0] ^
           LS_table[2][(o[1] >> (5 * 8)) & 0xFF][0] ^
           LS_table[3][(o[1] >> (4 * 8)) & 0xFF][0] ^
           LS_table[4][(o[1] >> (3 * 8)) & 0xFF][0] ^
           LS_table[5][(o[1] >> (2 * 8)) & 0xFF][0] ^
           LS_table[6][(o[1] >> (1 * 8)) & 0xFF][0] ^
           LS_table[7][(o[1] >> (0 * 8)) & 0xFF][0] ^
           LS_table[8][(o[0] >> (7 * 8)) & 0xFF][0] ^
           LS_table[9][(o[0] >> (6 * 8)) & 0xFF][0] ^
           LS_table[10][(o[0] >> (5 * 8)) & 0xFF][0] ^
           LS_table[11][(o[0] >> (4 * 8)) & 0xFF][0] ^
           LS_table[12][(o[0] >> (3 * 8)) & 0xFF][0] ^
           LS_table[13][(o[0] >> (2 * 8)) & 0xFF][0] ^
           LS_table[14][(o[0] >> (1 * 8)) & 0xFF][0] ^
           LS_table[15][(o[0] >> (0 * 8)) & 0xFF][0] ^ round_keys[2].kuzn_cast()[0];

    i[1] = LS_table[0][(o[1] >> (7 * 8)) & 0xFF][1] ^
           LS_table[1][(o[1] >> (6 * 8)) & 0xFF][1] ^
           LS_table[2][(o[1] >> (5 * 8)) & 0xFF][1] ^
           LS_table[3][(o[1] >> (4 * 8)) & 0xFF][1] ^
           LS_table[4][(o[1] >> (3 * 8)) & 0xFF][1] ^
           LS_table[5][(o[1] >> (2 * 8)) & 0xFF][1] ^
           LS_table[6][(o[1] >> (1 * 8)) & 0xFF][1] ^
           LS_table[7][(o[1] >> (0 * 8)) & 0xFF][1] ^
           LS_table[8][(o[0] >> (7 * 8)) & 0xFF][1] ^
           LS_table[9][(o[0] >> (6 * 8)) & 0xFF][1] ^
           LS_table[10][(o[0] >> (5 * 8)) & 0xFF][1] ^
           LS_table[11][(o[0] >> (4 * 8)) & 0xFF][1] ^
           LS_table[12][(o[0] >> (3 * 8)) & 0xFF][1] ^
           LS_table[13][(o[0] >> (2 * 8)) & 0xFF][1] ^
           LS_table[14][(o[0] >> (1 * 8)) & 0xFF][1] ^
           LS_table[15][(o[0] >> (0 * 8)) & 0xFF][1] ^ round_keys[2].kuzn_cast()[1];

    o[0] = LS_table[0][(i[1] >> (7 * 8)) & 0xFF][0] ^
           LS_table[1][(i[1] >> (6 * 8)) & 0xFF][0] ^
           LS_table[2][(i[1] >> (5 * 8)) & 0xFF][0] ^
           LS_table[3][(i[1] >> (4 * 8)) & 0xFF][0] ^
           LS_table[4][(i[1] >> (3 * 8)) & 0xFF][0] ^
           LS_table[5][(i[1] >> (2 * 8)) & 0xFF][0] ^
           LS_table[6][(i[1] >> (1 * 8)) & 0xFF][0] ^
           LS_table[7][(i[1] >> (0 * 8)) & 0xFF][0] ^
           LS_table[8][(i[0] >> (7 * 8)) & 0xFF][0] ^
           LS_table[9][(i[0] >> (6 * 8)) & 0xFF][0] ^
           LS_table[10][(i[0] >> (5 * 8)) & 0xFF][0] ^
           LS_table[11][(i[0] >> (4 * 8)) & 0xFF][0] ^
           LS_table[12][(i[0] >> (3 * 8)) & 0xFF][0] ^
           LS_table[13][(i[0] >> (2 * 8)) & 0xFF][0] ^
           LS_table[14][(i[0] >> (1 * 8)) & 0xFF][0] ^
           LS_table[15][(i[0] >> (0 * 8)) & 0xFF][0] ^ round_keys[3].kuzn_cast()[0];

    o[1] = LS_table[0][(i[1] >> (7 * 8)) & 0xFF][1] ^
           LS_table[1][(i[1] >> (6 * 8)) & 0xFF][1] ^
           LS_table[2][(i[1] >> (5 * 8)) & 0xFF][1] ^
           LS_table[3][(i[1] >> (4 * 8)) & 0xFF][1] ^
           LS_table[4][(i[1] >> (3 * 8)) & 0xFF][1] ^
           LS_table[5][(i[1] >> (2 * 8)) & 0xFF][1] ^
           LS_table[6][(i[1] >> (1 * 8)) & 0xFF][1] ^
           LS_table[7][(i[1] >> (0 * 8)) & 0xFF][1] ^
           LS_table[8][(i[0] >> (7 * 8)) & 0xFF][1] ^
           LS_table[9][(i[0] >> (6 * 8)) & 0xFF][1] ^
           LS_table[10][(i[0] >> (5 * 8)) & 0xFF][1] ^
           LS_table[11][(i[0] >> (4 * 8)) & 0xFF][1] ^
           LS_table[12][(i[0] >> (3 * 8)) & 0xFF][1] ^
           LS_table[13][(i[0] >> (2 * 8)) & 0xFF][1] ^
           LS_table[14][(i[0] >> (1 * 8)) & 0xFF][1] ^
           LS_table[15][(i[0] >> (0 * 8)) & 0xFF][1] ^ round_keys[3].kuzn_cast()[1];

    i[0] = LS_table[0][(o[1] >> (7 * 8)) & 0xFF][0] ^
           LS_table[1][(o[1] >> (6 * 8)) & 0xFF][0] ^
           LS_table[2][(o[1] >> (5 * 8)) & 0xFF][0] ^
           LS_table[3][(o[1] >> (4 * 8)) & 0xFF][0] ^
           LS_table[4][(o[1] >> (3 * 8)) & 0xFF][0] ^
           LS_table[5][(o[1] >> (2 * 8)) & 0xFF][0] ^
           LS_table[6][(o[1] >> (1 * 8)) & 0xFF][0] ^
           LS_table[7][(o[1] >> (0 * 8)) & 0xFF][0] ^
           LS_table[8][(o[0] >> (7 * 8)) & 0xFF][0] ^
           LS_table[9][(o[0] >> (6 * 8)) & 0xFF][0] ^
           LS_table[10][(o[0] >> (5 * 8)) & 0xFF][0] ^
           LS_table[11][(o[0] >> (4 * 8)) & 0xFF][0] ^
           LS_table[12][(o[0] >> (3 * 8)) & 0xFF][0] ^
           LS_table[13][(o[0] >> (2 * 8)) & 0xFF][0] ^
           LS_table[14][(o[0] >> (1 * 8)) & 0xFF][0] ^
           LS_table[15][(o[0] >> (0 * 8)) & 0xFF][0] ^ round_keys[4].kuzn_cast()[0];

    i[1] = LS_table[0][(o[1] >> (7 * 8)) & 0xFF][1] ^
           LS_table[1][(o[1] >> (6 * 8)) & 0xFF][1] ^
           LS_table[2][(o[1] >> (5 * 8)) & 0xFF][1] ^
           LS_table[3][(o[1] >> (4 * 8)) & 0xFF][1] ^
           LS_table[4][(o[1] >> (3 * 8)) & 0xFF][1] ^
           LS_table[5][(o[1] >> (2 * 8)) & 0xFF][1] ^
           LS_table[6][(o[1] >> (1 * 8)) & 0xFF][1] ^
           LS_table[7][(o[1] >> (0 * 8)) & 0xFF][1] ^
           LS_table[8][(o[0] >> (7 * 8)) & 0xFF][1] ^
           LS_table[9][(o[0] >> (6 * 8)) & 0xFF][1] ^
           LS_table[10][(o[0] >> (5 * 8)) & 0xFF][1] ^
           LS_table[11][(o[0] >> (4 * 8)) & 0xFF][1] ^
           LS_table[12][(o[0] >> (3 * 8)) & 0xFF][1] ^
           LS_table[13][(o[0] >> (2 * 8)) & 0xFF][1] ^
           LS_table[14][(o[0] >> (1 * 8)) & 0xFF][1] ^
           LS_table[15][(o[0] >> (0 * 8)) & 0xFF][1] ^ round_keys[4].kuzn_cast()[1];

    o[0] = LS_table[0][(i[1] >> (7 * 8)) & 0xFF][0] ^
           LS_table[1][(i[1] >> (6 * 8)) & 0xFF][0] ^
           LS_table[2][(i[1] >> (5 * 8)) & 0xFF][0] ^
           LS_table[3][(i[1] >> (4 * 8)) & 0xFF][0] ^
           LS_table[4][(i[1] >> (3 * 8)) & 0xFF][0] ^
           LS_table[5][(i[1] >> (2 * 8)) & 0xFF][0] ^
           LS_table[6][(i[1] >> (1 * 8)) & 0xFF][0] ^
           LS_table[7][(i[1] >> (0 * 8)) & 0xFF][0] ^
           LS_table[8][(i[0] >> (7 * 8)) & 0xFF][0] ^
           LS_table[9][(i[0] >> (6 * 8)) & 0xFF][0] ^
           LS_table[10][(i[0] >> (5 * 8)) & 0xFF][0] ^
           LS_table[11][(i[0] >> (4 * 8)) & 0xFF][0] ^
           LS_table[12][(i[0] >> (3 * 8)) & 0xFF][0] ^
           LS_table[13][(i[0] >> (2 * 8)) & 0xFF][0] ^
           LS_table[14][(i[0] >> (1 * 8)) & 0xFF][0] ^
           LS_table[15][(i[0] >> (0 * 8)) & 0xFF][0] ^ round_keys[5].kuzn_cast()[0];

    o[1] = LS_table[0][(i[1] >> (7 * 8)) & 0xFF][1] ^
           LS_table[1][(i[1] >> (6 * 8)) & 0xFF][1] ^
           LS_table[2][(i[1] >> (5 * 8)) & 0xFF][1] ^
           LS_table[3][(i[1] >> (4 * 8)) & 0xFF][1] ^
           LS_table[4][(i[1] >> (3 * 8)) & 0xFF][1] ^
           LS_table[5][(i[1] >> (2 * 8)) & 0xFF][1] ^
           LS_table[6][(i[1] >> (1 * 8)) & 0xFF][1] ^
           LS_table[7][(i[1] >> (0 * 8)) & 0xFF][1] ^
           LS_table[8][(i[0] >> (7 * 8)) & 0xFF][1] ^
           LS_table[9][(i[0] >> (6 * 8)) & 0xFF][1] ^
           LS_table[10][(i[0] >> (5 * 8)) & 0xFF][1] ^
           LS_table[11][(i[0] >> (4 * 8)) & 0xFF][1] ^
           LS_table[12][(i[0] >> (3 * 8)) & 0xFF][1] ^
           LS_table[13][(i[0] >> (2 * 8)) & 0xFF][1] ^
           LS_table[14][(i[0] >> (1 * 8)) & 0xFF][1] ^
           LS_table[15][(i[0] >> (0 * 8)) & 0xFF][1] ^ round_keys[5].kuzn_cast()[1];

    i[0] = LS_table[0][(o[1] >> (7 * 8)) & 0xFF][0] ^
           LS_table[1][(o[1] >> (6 * 8)) & 0xFF][0] ^
           LS_table[2][(o[1] >> (5 * 8)) & 0xFF][0] ^
           LS_table[3][(o[1] >> (4 * 8)) & 0xFF][0] ^
           LS_table[4][(o[1] >> (3 * 8)) & 0xFF][0] ^
           LS_table[5][(o[1] >> (2 * 8)) & 0xFF][0] ^
           LS_table[6][(o[1] >> (1 * 8)) & 0xFF][0] ^
           LS_table[7][(o[1] >> (0 * 8)) & 0xFF][0] ^
           LS_table[8][(o[0] >> (7 * 8)) & 0xFF][0] ^
           LS_table[9][(o[0] >> (6 * 8)) & 0xFF][0] ^
           LS_table[10][(o[0] >> (5 * 8)) & 0xFF][0] ^
           LS_table[11][(o[0] >> (4 * 8)) & 0xFF][0] ^
           LS_table[12][(o[0] >> (3 * 8)) & 0xFF][0] ^
           LS_table[13][(o[0] >> (2 * 8)) & 0xFF][0] ^
           LS_table[14][(o[0] >> (1 * 8)) & 0xFF][0] ^
           LS_table[15][(o[0] >> (0 * 8)) & 0xFF][0] ^ round_keys[6].kuzn_cast()[0];

    i[1] = LS_table[0][(o[1] >> (7 * 8)) & 0xFF][1] ^
           LS_table[1][(o[1] >> (6 * 8)) & 0xFF][1] ^
           LS_table[2][(o[1] >> (5 * 8)) & 0xFF][1] ^
           LS_table[3][(o[1] >> (4 * 8)) & 0xFF][1] ^
           LS_table[4][(o[1] >> (3 * 8)) & 0xFF][1] ^
           LS_table[5][(o[1] >> (2 * 8)) & 0xFF][1] ^
           LS_table[6][(o[1] >> (1 * 8)) & 0xFF][1] ^
           LS_table[7][(o[1] >> (0 * 8)) & 0xFF][1] ^
           LS_table[8][(o[0] >> (7 * 8)) & 0xFF][1] ^
           LS_table[9][(o[0] >> (6 * 8)) & 0xFF][1] ^
           LS_table[10][(o[0] >> (5 * 8)) & 0xFF][1] ^
           LS_table[11][(o[0] >> (4 * 8)) & 0xFF][1] ^
           LS_table[12][(o[0] >> (3 * 8)) & 0xFF][1] ^
           LS_table[13][(o[0] >> (2 * 8)) & 0xFF][1] ^
           LS_table[14][(o[0] >> (1 * 8)) & 0xFF][1] ^
           LS_table[15][(o[0] >> (0 * 8)) & 0xFF][1] ^ round_keys[6].kuzn_cast()[1];

    o[0] = LS_table[0][(i[1] >> (7 * 8)) & 0xFF][0] ^
           LS_table[1][(i[1] >> (6 * 8)) & 0xFF][0] ^
           LS_table[2][(i[1] >> (5 * 8)) & 0xFF][0] ^
           LS_table[3][(i[1] >> (4 * 8)) & 0xFF][0] ^
           LS_table[4][(i[1] >> (3 * 8)) & 0xFF][0] ^
           LS_table[5][(i[1] >> (2 * 8)) & 0xFF][0] ^
           LS_table[6][(i[1] >> (1 * 8)) & 0xFF][0] ^
           LS_table[7][(i[1] >> (0 * 8)) & 0xFF][0] ^
           LS_table[8][(i[0] >> (7 * 8)) & 0xFF][0] ^
           LS_table[9][(i[0] >> (6 * 8)) & 0xFF][0] ^
           LS_table[10][(i[0] >> (5 * 8)) & 0xFF][0] ^
           LS_table[11][(i[0] >> (4 * 8)) & 0xFF][0] ^
           LS_table[12][(i[0] >> (3 * 8)) & 0xFF][0] ^
           LS_table[13][(i[0] >> (2 * 8)) & 0xFF][0] ^
           LS_table[14][(i[0] >> (1 * 8)) & 0xFF][0] ^
           LS_table[15][(i[0] >> (0 * 8)) & 0xFF][0] ^ round_keys[7].kuzn_cast()[0];

    o[1] = LS_table[0][(i[1] >> (7 * 8)) & 0xFF][1] ^
           LS_table[1][(i[1] >> (6 * 8)) & 0xFF][1] ^
           LS_table[2][(i[1] >> (5 * 8)) & 0xFF][1] ^
           LS_table[3][(i[1] >> (4 * 8)) & 0xFF][1] ^
           LS_table[4][(i[1] >> (3 * 8)) & 0xFF][1] ^
           LS_table[5][(i[1] >> (2 * 8)) & 0xFF][1] ^
           LS_table[6][(i[1] >> (1 * 8)) & 0xFF][1] ^
           LS_table[7][(i[1] >> (0 * 8)) & 0xFF][1] ^
           LS_table[8][(i[0] >> (7 * 8)) & 0xFF][1] ^
           LS_table[9][(i[0] >> (6 * 8)) & 0xFF][1] ^
           LS_table[10][(i[0] >> (5 * 8)) & 0xFF][1] ^
           LS_table[11][(i[0] >> (4 * 8)) & 0xFF][1] ^
           LS_table[12][(i[0] >> (3 * 8)) & 0xFF][1] ^
           LS_table[13][(i[0] >> (2 * 8)) & 0xFF][1] ^
           LS_table[14][(i[0] >> (1 * 8)) & 0xFF][1] ^
           LS_table[15][(i[0] >> (0 * 8)) & 0xFF][1] ^ round_keys[7].kuzn_cast()[1];

    i[0] = LS_table[0][(o[1] >> (7 * 8)) & 0xFF][0] ^
           LS_table[1][(o[1] >> (6 * 8)) & 0xFF][0] ^
           LS_table[2][(o[1] >> (5 * 8)) & 0xFF][0] ^
           LS_table[3][(o[1] >> (4 * 8)) & 0xFF][0] ^
           LS_table[4][(o[1] >> (3 * 8)) & 0xFF][0] ^
           LS_table[5][(o[1] >> (2 * 8)) & 0xFF][0] ^
           LS_table[6][(o[1] >> (1 * 8)) & 0xFF][0] ^
           LS_table[7][(o[1] >> (0 * 8)) & 0xFF][0] ^
           LS_table[8][(o[0] >> (7 * 8)) & 0xFF][0] ^
           LS_table[9][(o[0] >> (6 * 8)) & 0xFF][0] ^
           LS_table[10][(o[0] >> (5 * 8)) & 0xFF][0] ^
           LS_table[11][(o[0] >> (4 * 8)) & 0xFF][0] ^
           LS_table[12][(o[0] >> (3 * 8)) & 0xFF][0] ^
           LS_table[13][(o[0] >> (2 * 8)) & 0xFF][0] ^
           LS_table[14][(o[0] >> (1 * 8)) & 0xFF][0] ^
           LS_table[15][(o[0] >> (0 * 8)) & 0xFF][0] ^ round_keys[8].kuzn_cast()[0];

    i[1] = LS_table[0][(o[1] >> (7 * 8)) & 0xFF][1] ^
           LS_table[1][(o[1] >> (6 * 8)) & 0xFF][1] ^
           LS_table[2][(o[1] >> (5 * 8)) & 0xFF][1] ^
           LS_table[3][(o[1] >> (4 * 8)) & 0xFF][1] ^
           LS_table[4][(o[1] >> (3 * 8)) & 0xFF][1] ^
           LS_table[5][(o[1] >> (2 * 8)) & 0xFF][1] ^
           LS_table[6][(o[1] >> (1 * 8)) & 0xFF][1] ^
           LS_table[7][(o[1] >> (0 * 8)) & 0xFF][1] ^
           LS_table[8][(o[0] >> (7 * 8)) & 0xFF][1] ^
           LS_table[9][(o[0] >> (6 * 8)) & 0xFF][1] ^
           LS_table[10][(o[0] >> (5 * 8)) & 0xFF][1] ^
           LS_table[11][(o[0] >> (4 * 8)) & 0xFF][1] ^
           LS_table[12][(o[0] >> (3 * 8)) & 0xFF][1] ^
           LS_table[13][(o[0] >> (2 * 8)) & 0xFF][1] ^
           LS_table[14][(o[0] >> (1 * 8)) & 0xFF][1] ^
           LS_table[15][(o[0] >> (0 * 8)) & 0xFF][1] ^ round_keys[8].kuzn_cast()[1];

    o[0] = LS_table[0][(i[1] >> (7 * 8)) & 0xFF][0] ^
           LS_table[1][(i[1] >> (6 * 8)) & 0xFF][0] ^
           LS_table[2][(i[1] >> (5 * 8)) & 0xFF][0] ^
           LS_table[3][(i[1] >> (4 * 8)) & 0xFF][0] ^
           LS_table[4][(i[1] >> (3 * 8)) & 0xFF][0] ^
           LS_table[5][(i[1] >> (2 * 8)) & 0xFF][0] ^
           LS_table[6][(i[1] >> (1 * 8)) & 0xFF][0] ^
           LS_table[7][(i[1] >> (0 * 8)) & 0xFF][0] ^
           LS_table[8][(i[0] >> (7 * 8)) & 0xFF][0] ^
           LS_table[9][(i[0] >> (6 * 8)) & 0xFF][0] ^
           LS_table[10][(i[0] >> (5 * 8)) & 0xFF][0] ^
           LS_table[11][(i[0] >> (4 * 8)) & 0xFF][0] ^
           LS_table[12][(i[0] >> (3 * 8)) & 0xFF][0] ^
           LS_table[13][(i[0] >> (2 * 8)) & 0xFF][0] ^
           LS_table[14][(i[0] >> (1 * 8)) & 0xFF][0] ^
           LS_table[15][(i[0] >> (0 * 8)) & 0xFF][0] ^ round_keys[9].kuzn_cast()[0];

    o[1] = LS_table[0][(i[1] >> (7 * 8)) & 0xFF][1] ^
           LS_table[1][(i[1] >> (6 * 8)) & 0xFF][1] ^
           LS_table[2][(i[1] >> (5 * 8)) & 0xFF][1] ^
           LS_table[3][(i[1] >> (4 * 8)) & 0xFF][1] ^
           LS_table[4][(i[1] >> (3 * 8)) & 0xFF][1] ^
           LS_table[5][(i[1] >> (2 * 8)) & 0xFF][1] ^
           LS_table[6][(i[1] >> (1 * 8)) & 0xFF][1] ^
           LS_table[7][(i[1] >> (0 * 8)) & 0xFF][1] ^
           LS_table[8][(i[0] >> (7 * 8)) & 0xFF][1] ^
           LS_table[9][(i[0] >> (6 * 8)) & 0xFF][1] ^
           LS_table[10][(i[0] >> (5 * 8)) & 0xFF][1] ^
           LS_table[11][(i[0] >> (4 * 8)) & 0xFF][1] ^
           LS_table[12][(i[0] >> (3 * 8)) & 0xFF][1] ^
           LS_table[13][(i[0] >> (2 * 8)) & 0xFF][1] ^
           LS_table[14][(i[0] >> (1 * 8)) & 0xFF][1] ^
           LS_table[15][(i[0] >> (0 * 8)) & 0xFF][1] ^ round_keys[9].kuzn_cast()[1];

    output_block.kuzn_cast()[0] = o[0];
    output_block.kuzn_cast()[1] = o[1];
}

void Kuznyechik::fast_encrypt_file(const std::string &in_file_name, const std::string &out_file_name) const {
    FILE *in_file_ptr = std::fopen(in_file_name.c_str(), "rb");
    FILE *out_file_ptr = std::fopen(out_file_name.c_str(), "wb");

    constexpr int buffer_size = 32768;
    uint8_t read_buffer[buffer_size];

    std::size_t cnt_read_bytes;
    ByteArray cur_in_block;
    while ((cnt_read_bytes = fread(read_buffer, sizeof(uint8_t), buffer_size, in_file_ptr)) > 0) {
        for (long long i = 0; i < buffer_size / INPUT_BLOCK_LENGTH; ++i) {
            cur_in_block.absorb(read_buffer + i * INPUT_BLOCK_LENGTH,
                                INPUT_BLOCK_LENGTH);
            fast_encrypt_block(cur_in_block, cur_in_block);
        }
        fwrite(read_buffer, sizeof(uint8_t), cnt_read_bytes, out_file_ptr);
    }
    cur_in_block.absorb(nullptr, 0);

    fclose(in_file_ptr);
    fclose(out_file_ptr);
}
