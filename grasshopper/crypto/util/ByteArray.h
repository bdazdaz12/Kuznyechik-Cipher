//#ifndef GRASSHOPPER_BYTEARRAY_H
//#define GRASSHOPPER_BYTEARRAY_H
//
//#include <cstdint>
//#include <cstdlib>
//
//class Block64Array {
//public:
//    Block64Array() {
//        byte_arr_ptr = nullptr;
//        arraySize = 0;
//    }
//
//    explicit Block64Array(std::size_t initial_size, uint8_t initial_value = 0);
//
//    Block64Array(const uint8_t *init_byte_arr_ptr, std::size_t init_arr_size);
//
//    // Move constructor
//    // Copy constructor thus implicitly deleted
//    // Object to move turn to null
//    Block64Array(Block64Array &&other) noexcept ;
//
//    Block64Arrayray();
//
//    void absorb(uint8_t *absorb_byte_arr_ptr, std::size_t absorb_arr_size);
//
//    // Move assigment operator
//    // Object to move turn to null
//    void operator=(Block64Array &&other) noexcept ;
//
//    uint8_t *get_byte_arr_ptr() {
//        return byte_arr_ptr;
//    }
//
//    uint8_t const *get_byte_arr_ptr() const {
//        return byte_arr_ptr;
//    }
//
//    uint64_t * kuzn_cast() const { // не const
//        return reinterpret_cast<uint64_t *>(const_cast<uint8_t *>(byte_arr_ptr));
//    }
//
//    uint8_t &operator[](size_t index);
//
//    uint8_t operator[](size_t index) const;
//
//    bool operator==(const Block64Array &other) const;
//
//    bool operator!=(const Block64Array &other) const;
//
//    void operator^=(const Block64Array &other);
//
//    Block64Array& operator=(const Block64Array &source);
//
//    size_t size() const {
//        return arraySize;
//    }
//
//    Block64Array create_copy() const;
//
//    void replace_array(const uint8_t *substitute_ptr, std::size_t substitute_size);
//
////    // It'll return slice of current ByteBlock
////    Block64Array operator()(size_t begin, size_t length) const;
//
//    /// lhs - Left Hand Side
//    /// rhs = Right Hand Side
//    friend void swap(Block64Array &lhs, Block64Array &rhs);
//
//private:
//    uint8_t *byte_arr_ptr;
//    std::size_t arraySize;
//};
//
//
//#endif //GRASSHOPPER_BYTEARRAY_H
