#include "ByteArray.h"

#include <memory.h>
#include <algorithm>
#include <stdexcept>
#include <iostream>

ByteArray &ByteArray::operator=(const ByteArray &other) {
    if (this != &other) {
        this->arraySize = other.arraySize;
        this->capacity = other.capacity;
        arrayPtr = std::shared_ptr<uint64_t[]>(new uint64_t[other.capacity], std::default_delete<uint64_t[]>());
        memcpy(arrayPtr.get(), other.arrayPtr.get(), other.size());
    }
    return *this;
}

uint8_t &ByteArray::operator[](size_t index) {
    return reinterpret_cast<uint8_t *>(arrayPtr.get())[index];
}

uint8_t ByteArray::operator[](size_t index) const {
    return reinterpret_cast<uint8_t *>(arrayPtr.get())[index];
}


void ByteArray::operator^=(const ByteArray &other) {
    for (int i = 0; i < std::min(this->capacity, other.capacity); ++i) {
        this->arrayPtr[i] ^= other.arrayPtr[i];
    }
}

void swap(ByteArray &lhs, ByteArray &rhs) {
    std::swap(lhs.arrayPtr, rhs.arrayPtr);
    std::swap(lhs.capacity, rhs.capacity);
    std::swap(lhs.arraySize, rhs.arraySize);
}

ByteArray::ByteArray(std::size_t initial_size, std::uint8_t initial_value) {
    arraySize = initial_size;
    capacity = arraySize / 8 + (arraySize % 8 == 0 ? 0 : 1);
    arrayPtr = std::shared_ptr<uint64_t[]>(new uint64_t[capacity], std::default_delete<uint64_t[]>());
    memset(arrayPtr.get(), initial_value, initial_size);
}

void ByteArray::print() const {
    std::cout << '[';
    auto *ptr = reinterpret_cast<uint8_t *>(arrayPtr.get());
    for (int i = 0; i < this->size(); ++i) {
        std::cout << (int) ptr[i];

        if (i != this->size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << ']' << std::endl;
}

ByteArray::ByteArray(const std::vector<uint64_t> &&v) {
    arraySize = v.size() * 8;
    capacity = v.size();
    arrayPtr = std::shared_ptr<uint64_t[]>(new uint64_t[v.size()], std::default_delete<uint64_t[]>());

    for (int i = 0; i < capacity; ++i) {
        arrayPtr[i] = v[i];
    }
}

void ByteArray::copyArrayInterval(const ByteArray &byteArray, std::size_t beginIdx, std::size_t endIdx) {
    arraySize = endIdx - beginIdx + 1;
    capacity = arraySize / 8 + arraySize % 8 == 0 ? 0 : 1;

    arrayPtr = std::shared_ptr<uint64_t[]>(new uint64_t[capacity], std::default_delete<uint64_t[]>());
    memcpy(this->arrayPtr.get(), byteArray.arrayPtr.get(), arraySize);
}


ByteArray::ByteArray(const ByteArray &&other) {
    this->arraySize = other.arraySize;
    this->capacity = other.capacity;
    this->arrayPtr = other.arrayPtr;
}

ByteArray::ByteArray(const ByteArray &other) {
    this->arraySize = other.arraySize;
    this->capacity = other.capacity;
    this->arrayPtr = std::shared_ptr<uint64_t[]>(new uint64_t[capacity], std::default_delete<uint64_t[]>());
    memcpy(this->arrayPtr.get(), other.arrayPtr.get(), other.size());
}

void ByteArray::operator=(ByteArray &&other) {
    if (this != &other) {
        this->arrayPtr = other.arrayPtr;
        this->arraySize = other.arraySize;
        this->capacity = other.capacity;
    }
}

ByteArray::ByteArray(const std::shared_ptr<uint8_t[]> &byteArrayPtr, std::size_t arraySize) {
    this->arraySize = arraySize;
    capacity = arraySize / 8 + (arraySize % 8 == 0 ? 0 : 1);
    arrayPtr = std::shared_ptr<uint64_t[]>(new uint64_t[capacity], std::default_delete<uint64_t[]>());
    memcpy(arrayPtr.get(), byteArrayPtr.get(), arraySize);
}

ByteArray ByteArray::operator^(const ByteArray &rhs) {
    ByteArray resultArray(this->size() > rhs.size()? *this : rhs);

    for (int i = 0; i < std::min(this->capacity, rhs.capacity); ++i) {
        resultArray.arrayPtr[i] = this->arrayPtr[i] ^ rhs.arrayPtr[i];
    }
    return resultArray;
}



