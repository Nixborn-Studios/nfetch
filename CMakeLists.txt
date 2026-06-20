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

target_compile_definitions(hwinfo_cpu PRIVATE HWINFO_DISABLE_WARNINGS=1)
target_compile_definitions(hwinfo_battery PRIVATE HWINFO_DISABLE_WARNINGS=1)
target_compile_definitions(hwinfo_network PRIVATE HWINFO_DISABLE_WARNINGS=1)
target_compile_definitions(hwinfo_os PRIVATE HWINFO_DISABLE_WARNINGS=1)
target_compile_definitions(hwinfo_ram PRIVATE HWINFO_DISABLE_WARNINGS=1)

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

if(MINGW OR UNIX)
    target_compile_options(nfetch PRIVATE
        -Wno-error=overflow
        -Wno-error=unknown-pragmas
        -Wno-error=pragmas
    )
endif()