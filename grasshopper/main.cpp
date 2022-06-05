#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <memory>

#include "crypto/kuznyechik/Kuznyechik.h"
#include "crypto/streebog/StreebogHash.h"

inline uint8_t from_hex_literal(char symbol) { // не оч функция канечна
    if (isdigit(symbol)) {
        return symbol - '0';
    } else if (symbol >= 'a' && symbol <= 'f') {
        return symbol - 'a' + 10;
    } else {
        return symbol - 'A' + 10;
    }
}

ByteArray hex_str_to_byte_arr(const std::string &hex_str) {
    if ((hex_str.length() & 1) != 0) {
        std::cerr << "HEX WARNING\n";
    }

    int size = hex_str.size() / 2;

    ByteArray result(size);
    for (int i = 0; i < hex_str.length() / 2; i++) {
        result[i] = from_hex_literal(hex_str[2 * i]) << 4;
        result[i] += from_hex_literal(hex_str[2 * i + 1]);
    }
    return result;
}

inline char to_hex_literal(uint8_t number) {
    if (number < 10) return '0' + number;
    if (number < 16) return 'a' + number - 10;
    throw std::invalid_argument("to_hex_literal: " + std::to_string(number));
}

std::string byte_arr_to_hex(const ByteArray &bb) {
    std::stringstream ss;
    for (int i = 0; i < bb.size(); i++) {
        ss << to_hex_literal(bb[i] >> 4);
        ss << to_hex_literal(bb[i] & 0xF);
    }
    std::string result;
    getline(ss, result);
    return result;
}

std::string byte_arr_to_hex(const uint8_t *arr, size_t arrSize) {
    std::stringstream ss;
    for (int i = 0; i < arrSize; i++) {
        ss << to_hex_literal(arr[i] >> 4);
        ss << to_hex_literal(arr[i] & 0xF);
    }
    std::string result;
    getline(ss, result);
    return result;
}

void init_read_buf(uint8_t read_buffer[], int buffer_size) {
    for (int i = 0; i < buffer_size; ++i) {
        read_buffer[i] = i % 256;
    }
}

void fill_file(const std::string &in_file_name) {
    FILE *out_file_ptr = std::fopen(in_file_name.c_str(), "wb");

    constexpr int buffer_size = 32768;
    uint8_t read_buffer[buffer_size];

    init_read_buf(read_buffer, buffer_size);

    for (int i = 0; i < 16000; ++i) {
        fwrite(read_buffer, sizeof(uint8_t), buffer_size, out_file_ptr);
    }
}


int main(int argc, char **argv) {
//    fill_file("in_file.txt");

    std::string in_file_name = "in_file.txt";
    FILE *in_file_ptr = std::fopen(in_file_name.c_str(), "rb");

    StreebogHash streebogHash(512);
    auto start = std::chrono::steady_clock::now();
    streebogHash.calculateHash(in_file_ptr);
    auto end = std::chrono::steady_clock::now();


    auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("Calculate HASH Time (sec): %lld\n", interval.count());
    std::cout << "FILE SIZE: 500 MB" << std::endl;


//    ByteArray secret_key = hex_str_to_byte_arr(
//            "8899aabbccddeeff0011223344556677fedcba98765432100123456789abcdef");
//
//
//    ByteArray in = hex_str_to_byte_arr("1122334455667700ffeeddccbbaa9988");
//
//    auto str = "012345678901234567890123456789012345678901234567890123456789012";
//    StreebogHash streebogHash(256);
//    std::cout << streebogHash.convertToHex(streebogHash.calculateHash((unsigned char *) str)) << std::endl;
//    std::cout << streebogHash.convertToHex(streebogHash.calculateHash(in)) << std::endl;
//    std::cout << streebogHash.convertToHex(streebogHash.calculateHash(secret_key)) << std::endl;
//    std::cout << streebogHash.convertToHex(streebogHash.calculateHash((unsigned char *) str)) << std::endl;
//
//
//    in.print();

//    Kuznyechik kuznyechik_instance(secret_key);
//
//    ByteArray in = hex_str_to_byte_arr("1122334455667700ffeeddccbbaa9988");
//    in.print();
//    ByteArray out;
//
//    auto start = std::chrono::steady_clock::now();
//    kuznyechik_instance.encrypt_block(in, out);
//    auto end = std::chrono::steady_clock::now();
//
//    std::cout << byte_arr_to_hex(out);

//
//    auto interval = std::chrono::duration_cast<std::chrono::seconds>(end - start);
//    printf("Encryption Time: %lld\n", interval.count());
//
//
////    kuznyechik_instance.decrypt_file("out_file.txt", "out_file_1.txt");



//    std::string str = "77fedcba98765432100123456789abcd";
//    ByteArray in = hex_str_to_byte_arr(str);
//    ByteArray out;
////    kuznyechik_instance.encrypt_block(in, out);
//    kuznyechik_instance.encrypt_block(in, in);
//
//    kuznyechik_instance.decrypt_block(in, in);
//
//    std::cout << byte_arr_to_hex(in);
}
