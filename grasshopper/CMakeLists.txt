cmake_minimum_required(VERSION 3.20)
project(grasshopper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O2")

add_executable(grasshopper main.cpp crypto/kuznyechik/Kuznyechik.cpp crypto/kuznyechik/Kuznyechik.h crypto/util/ByteArray.cpp crypto/util/ByteArray.h crypto/kuznyechik/CipherConstants.h crypto/kuznyechik/KuznyechickLUT.h crypto/kuznyechik/util/Block64Array.h crypto/kuznyechik/util/Block64Array.cpp)
