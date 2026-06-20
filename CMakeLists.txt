cmake_minimum_required(VERSION 3.20)
project(nfetch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(nfetch
    src/main.cpp
)

target_include_directories(nfetch PRIVATE
    src
    vendor/stb
)

# MSYS2 / MinGW FIXES
if(MINGW OR MSYS OR WIN32)
    add_compile_options(
        -Wno-error
        -Wno-unknown-pragmas
        -Wno-pragmas
    )
endif()
# VENDOR
add_subdirectory(vendor/hwinfo)

if(MINGW OR MSYS)
    foreach(tgt hwinfo hwinfo_cpu hwinfo_battery)
        if(TARGET ${tgt})
            target_compile_options(${tgt} PRIVATE
                -Wno-error
                -Wno-error=overflow
                -Wno-unknown-pragmas
            )
        endif()
    endforeach()
endif()

target_link_libraries(nfetch PRIVATE hwinfo)

# PkgConfig (Linux/MSYS2)
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