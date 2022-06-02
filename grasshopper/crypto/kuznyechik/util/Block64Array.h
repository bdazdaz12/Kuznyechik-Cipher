#ifndef GRASSHOPPER_BYTEARRAY_H
#define GRASSHOPPER_BYTEARRAY_H

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>

class Block64Array {
public:

    Block64Array() = default;

    Block64Array(const std::vector<uint64_t> &v);

    Block64Array(const std::vector<uint64_t> &&v);

    explicit Block64Array(std::size_t initial_size, uint64_t initial_value = 0);

    uint64_t &operator[](size_t index);

    uint64_t operator[](size_t index) const;

    void operator^=(const Block64Array &other);

    Block64Array& operator=(const Block64Array &source);

    size_t size() const {
        return array.size();
    }

    uint8_t getByteAt(size_t index);

    /// lhs - Left Hand Side
    /// rhs = Right Hand Side
    friend void swap(Block64Array &lhs, Block64Array &rhs);

    void print();

private:
    std::vector<uint64_t> array;
};


#endif //GRASSHOPPER_BYTEARRAY_H
