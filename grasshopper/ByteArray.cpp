#include "ByteArray.h"

#include <memory.h>
#include <algorithm>

ByteArray::ByteArray(const size_t initial_size, const uint8_t initial_value)
        : byte_arr_size(initial_size) {
    if (initial_size == 0) {
        byte_arr_ptr = nullptr;
    } else {
        byte_arr_ptr = new uint8_t[initial_size];
        memset(byte_arr_ptr, initial_value, initial_size);
    }
}

ByteArray::ByteArray(const uint8_t *const init_byte_arr_ptr, const size_t init_arr_size)
        : byte_arr_size(init_arr_size) {
    byte_arr_ptr = new uint8_t[init_arr_size];
    memcpy(byte_arr_ptr, init_byte_arr_ptr, init_arr_size);
}

ByteArray::ByteArray(ByteArray &&other)
        : byte_arr_ptr(other.byte_arr_ptr), byte_arr_size(other.byte_arr_size) {
    other.byte_arr_ptr = nullptr; /// чтобы деструктор не сломал память
    other.byte_arr_size = 0;
}

ByteArray::~ByteArray() {
    delete[] byte_arr_ptr; // nullptr не ломает delete
}

void ByteArray::operator=(ByteArray &&other) {
    if (this == &other) {
        return;
    }
    delete[] byte_arr_ptr;

    byte_arr_ptr = other.byte_arr_ptr;
    byte_arr_size = other.byte_arr_size;

    other.byte_arr_ptr = nullptr;
    other.byte_arr_size = 0;
}

uint8_t &ByteArray::operator[](size_t index) {
    return byte_arr_ptr[index];
}

uint8_t ByteArray::operator[](size_t index) const {
    return byte_arr_ptr[index];
}

bool ByteArray::operator==(const ByteArray &other) const {
    if (this->byte_arr_size != other.byte_arr_size) {
        return false;
    } else if (this->byte_arr_ptr == other.byte_arr_ptr) {
        return true;
    } else {
        bool is_equal = true;
        for (int i = 0; i < this->byte_arr_size; ++i) {
            if (this->byte_arr_ptr[i] != other.byte_arr_ptr[i]) {
                is_equal = false;
                break;
            }
        }
        return is_equal;
    }
}

bool ByteArray::operator!=(const ByteArray &other) const {
    return !(*this == other);
}

ByteArray ByteArray::create_copy() const {
    return ByteArray(byte_arr_ptr, byte_arr_size);
}

void ByteArray::operator^=(const ByteArray &other) {
    for (int i = 0; i < std::min(this->byte_arr_size, other.byte_arr_size); ++i) {
        this->byte_arr_ptr[i] ^= other.byte_arr_ptr[i];
    }
}

void ByteArray::replace_array(const uint8_t *substitute_ptr, size_t substitute_size) {
    delete[] byte_arr_ptr;

    if (substitute_size > 0 && substitute_ptr != nullptr) {
        byte_arr_ptr = new uint8_t[substitute_size];
        memcpy(byte_arr_ptr, substitute_ptr, substitute_size);
        byte_arr_size = substitute_size;
    } else {
        byte_arr_ptr = nullptr;
        byte_arr_size = 0;
    }
}

ByteArray &ByteArray::operator=(const ByteArray &source) {
    if (this != &source) {
        if (this->byte_arr_size == source.byte_arr_size) {
            memcpy(byte_arr_ptr, source.get_byte_arr_ptr(), source.byte_arr_size);
        } else {
            this->replace_array(source.get_byte_arr_ptr(), source.byte_arr_size);
        }
    }
    return *this;
}
