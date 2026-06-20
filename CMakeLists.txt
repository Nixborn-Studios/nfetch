cmake_minimum_required(VERSION 3.20)
project(nfetch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_executable(nfetch
    src/main.cpp
)

target_include_directories(nfetch PRIVATE
    src
    vendor/stb
)

add_subdirectory(vendor/hwinfo)

target_compile_options(hwinfo_battery PRIVATE
    -Wno-error
    -Wno-unknown-pragmas
    -Wno-pragmas
    -Wno-overflow
    -Wno-error=overflow
)

target_link_libraries(nfetch PRIVATE
    lfreist-hwinfo::hwinfo
)

# Linux + MSYS2 shared pkg-config stack
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(CHAFA REQUIRED chafa)

target_include_directories(nfetch PRIVATE
    ${GLIB_INCLUDE_DIRS}
    ${CHAFA_INCLUDE_DIRS}
)

target_link_libraries(nfetch PRIVATE
    ${GLIB_LIBRARIES}
    ${CHAFA_LIBRARIES}
)