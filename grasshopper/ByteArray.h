#ifndef GRASSHOPPER_BYTEARRAY_H
#define GRASSHOPPER_BYTEARRAY_H

#include <cstdint>

class ByteArray {
public:
    ByteArray() {
        byte_arr_ptr = nullptr;
        byte_arr_size = 0;
    }

    ByteArray(size_t initial_size, uint8_t initial_value = 0);

    ByteArray(const uint8_t *init_byte_arr_ptr, size_t init_arr_size);

    // Move constructor
    // Copy constructor thus implicitly deleted
    // Object to move turn to null
    ByteArray(ByteArray &&other);

    ~ByteArray();

    void absorb(uint8_t *absorb_byte_arr_ptr, size_t absorb_arr_size);

    // Move assigment operator
    // Object to move turn to null
    void operator=(ByteArray &&other);

    uint8_t *get_byte_arr_ptr() {
        return byte_arr_ptr;
    }

    uint8_t const *get_byte_arr_ptr() const {
        return byte_arr_ptr;
    }

    uint64_t * kuzn_cast() const { // не const
        return reinterpret_cast<uint64_t *>(const_cast<uint8_t *>(byte_arr_ptr));
    }

    uint8_t &operator[](size_t index);

    uint8_t operator[](size_t index) const;

    bool operator==(const ByteArray &other) const;

    bool operator!=(const ByteArray &other) const;

    void operator^=(const ByteArray &other);

    ByteArray& operator=(const ByteArray &source);

    size_t size() const {
        return byte_arr_size;
    }

    ByteArray create_copy() const;

    void replace_array(const uint8_t *substitute_ptr, size_t substitute_size);

//    // It'll return slice of current ByteBlock
//    ByteArray operator()(size_t begin, size_t length) const;

    /// lhs - Left Hand Side
    /// rhs = Right Hand Side
    friend void swap(ByteArray &lhs, ByteArray &rhs);

private:
    uint8_t *byte_arr_ptr;
    size_t byte_arr_size;
};


#endif //GRASSHOPPER_BYTEARRAY_H
