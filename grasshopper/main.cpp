#include <iostream>
#include <fstream>
#include <sstream>

#include "Kuznyechik.h"
#include "ByteArray.h"

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
    for (int i = 0; i < size; i++) {
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

int main(int argc, char **argv) {
    ByteArray secret_key = hex_str_to_byte_arr(
            "8899aabbccddeeff0011223344556677fedcba98765432100123456789abcdef");

    Kuznyechik kuznyechik_instance(secret_key);

    std::string str = "77fedcba98765432100123456789abcd";
    ByteArray in = hex_str_to_byte_arr(str);
    ByteArray out;
    kuznyechik_instance.encrypt_block(in, out);

    kuznyechik_instance.decrypt_block(out, in);

    std::cout << byte_arr_to_hex(in);
}
