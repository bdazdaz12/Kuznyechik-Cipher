cmake_minimum_required(VERSION 3.20)
project(grasshopper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O2")

add_executable(grasshopper main.cpp Kuznyechik.cpp Kuznyechik.h ByteArray.cpp ByteArray.h KuznyechikConstants.h KuznyechickLUT.h)
