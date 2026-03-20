cmake_minimum_required(VERSION 3.22)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(blinking_led)

target_link_libraries(blinking_led.elf PRIVATE 
    ${CMAKE_BINARY_DIR}/esp-idf/xtensa/libxtensa.a)
