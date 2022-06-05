#pragma once

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>
#include <iostream>

class ByteArray {
public:
    ByteArray() {
        capacity = 0;
        arraySize = 0;
    };

    ByteArray(const std::vector<uint64_t> &&v);

    ByteArray(const std::shared_ptr<uint8_t[]> &byteArrayPtr, std::size_t arraySize);

    ByteArray(const ByteArray &&other);

    ByteArray(const ByteArray &other);

    ByteArray(std::size_t initial_size, uint8_t initial_value = 0);

    ~ByteArray() = default;

    uint8_t &operator[](size_t index);

    uint8_t operator[](size_t index) const;

    void operator^=(const ByteArray &other);

    ByteArray& operator=(const ByteArray &source);

    void operator=(ByteArray &&other);

    ByteArray operator^(const ByteArray &rhs);

    size_t size() const {
        return arraySize;
    }

    /// lhs - Left Hand Side
    /// rhs = Right Hand Side
    friend void swap(ByteArray &lhs, ByteArray &rhs);

    void print() const;

    void copyArrayInterval(const ByteArray &byteArray, std::size_t beginIdx, std::size_t endIdx);

    const uint8_t * getBytes() const {
        return reinterpret_cast<const uint8_t * const>(arrayPtr.get());
    }

    const uint64_t *getQwords() const {
        return reinterpret_cast<const uint64_t * const>(arrayPtr.get());
    }

    uint64_t *getMemoryPtr() {
        return arrayPtr.get();
    }

private:
    std::shared_ptr<uint64_t[]> arrayPtr;
    size_t capacity{}; ///@details сколько uint64_t выделенно под хранение
    size_t arraySize{}; /// количество байт в массиве
};
