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

target_link_libraries(nfetch PRIVATE
    lfreist-hwinfo::hwinfo
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(CHAFA chafa)

if(CHAFA_FOUND)
    target_include_directories(nfetch PRIVATE ${CHAFA_INCLUDE_DIRS})
    target_link_libraries(nfetch PRIVATE ${CHAFA_LIBRARIES})
endif()

target_include_directories(nfetch PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(nfetch PRIVATE ${GLIB_LIBRARIES})

if(MINGW OR MSYS OR WIN32)
    add_compile_options(
        -Wno-error=overflow
        -Wno-error=unknown-pragmas
    )
endif()

install(TARGETS nfetch RUNTIME DESTINATION .)
install(DIRECTORY assets/ DESTINATION assets)