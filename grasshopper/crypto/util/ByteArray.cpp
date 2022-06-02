//#include "Block64Array.h"
//
//#include <memory.h>
//#include <algorithm>
//#include <stdexcept>
//
//Block64Array::Block64Array(const size_t initial_size, const uint8_t initial_value)
//        : arraySize(initial_size) {
//    if (initial_size == 0) {
//        byte_arr_ptr = nullptr;
//    } else {
//        byte_arr_ptr = new uint8_t[initial_size];
//        memset(byte_arr_ptr, initial_value, initial_size);
//    }
//}
//
//Block64Array::Block64Array(const uint8_t *const init_byte_arr_ptr, const size_t init_arr_size)
//        : arraySize(init_arr_size) {
//    byte_arr_ptr = new uint8_t[init_arr_size];
//    memcpy(byte_arr_ptr, init_byte_arr_ptr, init_arr_size);
//}
//
//Block64Array::Block64Array(Block64Array &&other)
//        : byte_arr_ptr(other.byte_arr_ptr), arraySize(other.arraySize) {
//    other.byte_arr_ptr = nullptr; /// чтобы деструктор не сломал память
//    other.arraySize = 0;
//}
//
//Block64Array::Block64Arrayray() {
//    delete[] byte_arr_ptr; // nullptr не ломает delete
//}
//
//void Block64Array::operator=(Block64Array &&other) {
//    if (this == &other) {
//        return;
//    }
//    delete[] byte_arr_ptr;
//
//    byte_arr_ptr = other.byte_arr_ptr;
//    arraySize = other.arraySize;
//
//    other.byte_arr_ptr = nullptr;
//    other.arraySize = 0;
//}
//
//uint8_t &Block64Array::operator[](size_t index) {
//    return byte_arr_ptr[index];
//}
//
//uint8_t Block64Array::operator[](size_t index) const {
//    return byte_arr_ptr[index];
//}
//
//bool Block64Array::operator==(const Block64Array &other) const {
//    if (this->arraySize != other.arraySize) {
//        return false;
//    } else if (this->byte_arr_ptr == other.byte_arr_ptr) {
//        return true;
//    } else {
//        bool is_equal = true;
//        for (int i = 0; i < this->arraySize; ++i) {
//            if (this->byte_arr_ptr[i] != other.byte_arr_ptr[i]) {
//                is_equal = false;
//                break;
//            }
//        }
//        return is_equal;
//    }
//}
//
//bool Block64Array::operator!=(const Block64Array &other) const {
//    return !(*this == other);
//}
//
//Block64Array Block64Array::create_copy() const {
//    return Block64Array(byte_arr_ptr, arraySize);
//}
//
//void Block64Array::operator^=(const Block64Array &other) {
//    for (int i = 0; i < std::min(this->arraySize, other.arraySize); ++i) {
//        this->byte_arr_ptr[i] ^= other.byte_arr_ptr[i];
//    }
//}
//
//void Block64Array::replace_array(const uint8_t *substitute_ptr, size_t substitute_size) {
//    delete[] byte_arr_ptr;
//
//    if (substitute_size > 0 && substitute_ptr != nullptr) {
//        byte_arr_ptr = new uint8_t[substitute_size];
//        memcpy(byte_arr_ptr, substitute_ptr, substitute_size);
//        arraySize = substitute_size;
//    } else {
//        byte_arr_ptr = nullptr;
//        arraySize = 0;
//    }
//}
//
//Block64Array &Block64Array::operator=(const Block64Array &source) {
//    if (this != &source) {
//        if (this->arraySize == source.arraySize) {
//            memcpy(byte_arr_ptr, source.get_byte_arr_ptr(), source.arraySize);
//        } else {
//            this->replace_array(source.get_byte_arr_ptr(), source.arraySize);
//        }
//    }
//    return *this;
//}
//
//void Block64Array::absorb(uint8_t *absorb_byte_arr_ptr, size_t absorb_arr_size) {
////    if (byte_arr_ptr != nullptr) { // костыльчик
////        throw std::invalid_argument("Block64Array.absorb(): prev array state is not null\n");
////    }
//    byte_arr_ptr = absorb_byte_arr_ptr;
//    arraySize = absorb_arr_size;
//}
