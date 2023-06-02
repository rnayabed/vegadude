#[[
Copyright (C) 2023 Debayan Sutradhar (rnayabed) (debayansutradhar3@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
]]

cmake_minimum_required(VERSION 3.5)

project(vegadude LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(vegadude
    logger.h logger.cpp
    main.cpp
    crc.h crc.cpp
    device.h device.cpp
    serialdevice.h serialdevice.cpp
    xmodem.h xmodem.cpp)

add_compile_definitions(
    VERSION=1.0
    LICENSE="https://github.com/rnayabed/vegadude/blob/master/LICENSE"
    ARIES_XMODEM_BLOCK_SIZE=128
)

target_compile_options(vegadude PRIVATE
    -Wall -Wextra -Werror

    $<$<CONFIG:Debug>:-O0 -ggdb3>
    $<$<CONFIG:Release>:-Os -Wl,--gc-sections -fno-builtin-printf
    -fno-builtin-puts -fdata-sections -ffunction-sections -fno-builtin-memcmp >
    $<$<CONFIG:MinSizeRel>:-Os>
    $<$<CONFIG:RelWithDebInfo>:-Og -ggdb>
)

install(TARGETS vegadude
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
