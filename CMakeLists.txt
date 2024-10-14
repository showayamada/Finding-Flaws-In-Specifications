cmake_minimum_required(VERSION 3.10)
project(ffs)

# C++のバージョンを指定
set(CMAKE_CXX_STANDARD 17)

# gcc 指定
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
# コンパイルオプション
set(CMAKE_CXX_FLAGS "-pthread")

# Spotのインクルードパスとライブラリパスを指定
include(config.cmake)
include_directories("${SPOT_PATH}/include")
link_directories("${SPOT_PATH}/lib")

# ソースファイルの指定
add_executable(ffs src/main.cpp)

# Spotのライブラリをリンク
target_link_libraries(ffs spot)
target_link_libraries(ffs bddx)
