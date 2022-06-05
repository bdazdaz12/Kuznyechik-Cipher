cmake_minimum_required(VERSION 3.20)
project(grasshopper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O2")

add_executable(grasshopper main.cpp crypto/kuznyechik/Kuznyechik.cpp crypto/kuznyechik/Kuznyechik.h crypto/kuznyechik/CipherConstants.h crypto/kuznyechik/KuznyechickLUT.h crypto/util/ByteArray.h crypto/util/ByteArray.cpp "crypto/streebog/streebog-enter.h" "crypto/streebog/streebog-consts.h" "crypto/streebog/streebog-core.h" "crypto/streebog/streebog-precalc.h" "crypto/streebog/streebog-ref.h" "crypto/streebog/StreebogHash.cpp" "crypto/streebog/StreebogHash.h" "crypto/streebog/streebog-config.h")
