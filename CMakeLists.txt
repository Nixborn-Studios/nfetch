cmake_minimum_required(VERSION 3.22)
project(nfetch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()

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
pkg_check_modules(CHAFA REQUIRED chafa)

target_include_directories(nfetch PRIVATE
    ${GLIB_INCLUDE_DIRS}
    ${CHAFA_INCLUDE_DIRS}
)

target_link_libraries(nfetch PRIVATE
    ${GLIB_LIBRARIES}
    ${CHAFA_LIBRARIES}
)

set(INSTALLER_PATH
    $<IF:$<CONFIG:Release>,
        ${CMAKE_BINARY_DIR}/Release/nfetch_installer.exe,
        ${CMAKE_BINARY_DIR}/nfetch_installer.exe
    >
)

add_custom_target(package_zip ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory package
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nfetch> package/nfetch.exe
    COMMAND ${CMAKE_COMMAND} -E copy ${INSTALLER_PATH} package/installer.exe
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets package/assets
    COMMAND ${CMAKE_COMMAND} -E tar "cfv" nfetch.zip --format=zip package
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)