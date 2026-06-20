cmake_minimum_required(VERSION 3.22)
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

add_subdirectory(vendor/hwinfo)

target_link_libraries(nfetch PRIVATE lfreist-hwinfo::hwinfo)

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)

    pkg_check_modules(GLIB QUIET glib-2.0)
    if(GLIB_FOUND)
        target_include_directories(nfetch PRIVATE ${GLIB_INCLUDE_DIRS})
        target_link_libraries(nfetch PRIVATE ${GLIB_LIBRARIES})
    endif()

    pkg_check_modules(CHAFA QUIET chafa)
    if(CHAFA_FOUND)
        target_include_directories(nfetch PRIVATE ${CHAFA_INCLUDE_DIRS})
        target_link_libraries(nfetch PRIVATE ${CHAFA_LIBRARIES})
    endif()

endif()

if(MINGW OR MSYS OR WIN32)
    add_compile_options(
        -Wno-error
        -Wno-unknown-pragmas
        -Wno-pragmas
        -Wno-missing-field-initializers
        -Wno-error=overflow
    )
endif()

if(MSVC)
    add_compile_options(/W4)
endif()