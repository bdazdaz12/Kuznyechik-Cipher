#pragma once

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>
#include <iostream>

class ByteArray {
public:
    ByteArray() {
        arrayPtr = new uint64_t[1];
        capacity = 1;
        arraySize = 0;
    };

    ByteArray(const std::vector<uint64_t> &&v);

    ByteArray(const ByteArray &&other);

    ByteArray(const ByteArray &other);


    ByteArray(std::size_t initial_size, uint8_t initial_value = 0);


    ~ByteArray() {
        delete[](arrayPtr);
    }

    uint8_t &operator[](size_t index);

    uint8_t operator[](size_t index) const;

    void operator^=(const ByteArray &other);

    ByteArray& operator=(const ByteArray &source);

//    void operator=(ByteArray &&other);

    size_t size() const {
        return arraySize;
    }

    /// lhs - Left Hand Side
    /// rhs = Right Hand Side
    friend void swap(ByteArray &lhs, ByteArray &rhs);

    void print() const;

    void copyArrayInterval(const ByteArray &byteArray, std::size_t beginIdx, std::size_t endIdx);

    uint8_t *getArrayPtr() const {
        return reinterpret_cast<uint8_t * const>(arrayPtr);
    }

private:
    uint64_t *arrayPtr;
    size_t capacity{}; ///@details сколько uint64_t выделенно под хранение
    size_t arraySize{}; /// количество байт в массиве
};
