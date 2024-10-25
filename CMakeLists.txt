cmake_minimum_required(VERSION 3.10)
project(ffs)

# C++のバージョンを指定
set(CMAKE_CXX_STANDARD 17)

# gcc 指定
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
# コンパイルオプション
set(CMAKE_CXX_FLAGS "-pthread")

# インクルードパスとライブラリパスを指定
include(config.cmake)
include_directories("${LIBRARY_PATH}/include")
link_directories("${LIBRARY_PATH}/lib")


# ソースファイルの指定
add_executable(ffs src/main.cpp)

# ライブラリをリンク
target_link_libraries(ffs spot)
target_link_libraries(ffs bddx)
target_link_libraries(ffs boost_atomic)
target_link_libraries(ffs boost_chrono)
target_link_libraries(ffs boost_charconv)
target_link_libraries(ffs boost_container)
target_link_libraries(ffs boost_context)
target_link_libraries(ffs boost_contract)
target_link_libraries(ffs boost_coroutine)
target_link_libraries(ffs boost_date_time)
target_link_libraries(ffs boost_exception)
target_link_libraries(ffs boost_fiber)
target_link_libraries(ffs boost_filesystem)
target_link_libraries(ffs boost_graph)
target_link_libraries(ffs boost_iostreams)
target_link_libraries(ffs boost_json)
target_link_libraries(ffs boost_locale)
target_link_libraries(ffs boost_log_setup)
target_link_libraries(ffs boost_log)
target_link_libraries(ffs boost_nowide)
target_link_libraries(ffs boost_prg_exec_monitor)
target_link_libraries(ffs boost_process)
target_link_libraries(ffs boost_program_options)
target_link_libraries(ffs boost_random)
target_link_libraries(ffs boost_regex)
target_link_libraries(ffs boost_serialization)
target_link_libraries(ffs boost_stacktrace_addr2line)
target_link_libraries(ffs boost_stacktrace_basic)
target_link_libraries(ffs boost_stacktrace_noop)
target_link_libraries(ffs boost_system)
target_link_libraries(ffs boost_thread)
target_link_libraries(ffs boost_timer)
target_link_libraries(ffs boost_type_erasure)
target_link_libraries(ffs boost_unit_test_framework)
target_link_libraries(ffs boost_wave)
target_link_libraries(ffs boost_wserialization)
target_link_libraries(ffs boost_test_exec_monitor)
target_link_libraries(ffs boost_url)