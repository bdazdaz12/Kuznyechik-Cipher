#include "Block64Array.h"

#include <memory.h>
#include <algorithm>
#include <stdexcept>
#include <iostream>

Block64Array &Block64Array::operator=(const Block64Array &other) {
    if (this != &other) {
        array = other.array;
    }
}

uint64_t &Block64Array::operator[](size_t index) {
    return array[index];
}

uint64_t Block64Array::operator[](size_t index) const {
    return array[index];
}


void Block64Array::operator^=(const Block64Array &other) {
    for (int i = 0; i < std::min(this->array.size(), other.array.size()); ++i) {
        this->array[i] ^= other.array[i];
    }
}

void swap(Block64Array &lhs, Block64Array &rhs) {
    swap(lhs.array, rhs.array);
}

Block64Array::Block64Array(std::size_t initial_size, uint64_t initial_value) {
    array = std::vector<uint64_t>(initial_size, initial_value);
}

void Block64Array::print() {
    std::cout << '[';
    for (int i = 0; i < this->size(); ++i) {
        std::cout << this->array[i];

        if (i != this->size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << ']' << std::endl;
}

uint8_t Block64Array::getByteAt(size_t index) {
    return (array[index / 8] >> (index % 8 * 8)) & 255u;
}

Block64Array::Block64Array(const std::vector<uint64_t> &v) {
    this->array = v;
}

Block64Array::Block64Array(const std::vector<uint64_t> &&v) {
    this->array = v;
}
