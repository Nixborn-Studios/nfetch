cmake_minimum_required(VERSION 3.20)
project(nfetch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MAIN APP
add_executable(nfetch
    src/main.cpp
)

target_include_directories(nfetch PRIVATE
    src
    vendor/stb
)

# HWINFO
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(HWINFO_SHARED OFF CACHE BOOL "" FORCE)
set(HWINFO_STATIC ON CACHE BOOL "" FORCE)

add_subdirectory(vendor/hwinfo)

target_link_libraries(nfetch PRIVATE
    lfreist-hwinfo::hwinfo
)

# LINUX SYSTEM DEPS ONLY
if (UNIX AND NOT APPLE)

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(GLIB REQUIRED glib-2.0)
    pkg_check_modules(CHAFA REQUIRED chafa)

    target_include_directories(nfetch PRIVATE
        ${GLIB_INCLUDE_DIRS}
        ${CHAFA_INCLUDE_DIRS}
    )

    target_link_directories(nfetch PRIVATE
        ${GLIB_LIBRARY_DIRS}
        ${CHAFA_LIBRARY_DIRS}
    )

    target_link_libraries(nfetch PRIVATE
        ${GLIB_LIBRARIES}
        ${CHAFA_LIBRARIES}
    )

endif()

# INSTALLER
add_executable(nfetch_installer
    src/installer.cpp
)