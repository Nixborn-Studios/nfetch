cmake_minimum_required(VERSION 3.22)
project(nfetch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(nfetch src/main.cpp)

target_include_directories(nfetch PRIVATE
    src
    vendor/stb
)

set(HWINFO_SHARED OFF CACHE BOOL "" FORCE)
set(HWINFO_STATIC ON CACHE BOOL "" FORCE)

add_subdirectory(vendor/hwinfo)

target_link_libraries(nfetch PRIVATE lfreist-hwinfo::hwinfo)

if(MINGW)
    foreach(t hwinfo_cpu hwinfo_battery hwinfo_gpu hwinfo_disk hwinfo_mainboard hwinfo_os hwinfo_ram hwinfo_network)
        if(TARGET ${t})
            target_compile_options(${t} PRIVATE -Wno-error=unknown-pragmas -Wno-error=pragmas)
        endif()
    endforeach()
endif()

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
        -Wno-error
        -Wno-unknown-pragmas
        -Wno-pragmas
        -Wno-missing-field-initializers
    )
endif()