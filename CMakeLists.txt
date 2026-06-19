cmake_minimum_required(VERSION 3.20)
project(nfetch)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(nfetch
    src/main.cpp
)
target_include_directories(nfetch PRIVATE src vendor/stb)

add_subdirectory(vendor/hwinfo)
target_link_libraries(nfetch PRIVATE lfreist-hwinfo::hwinfo)

# Chofa

target_include_directories(nfetch PRIVATE
    ${CHAFA_INCLUDE_DIRS}
)

target_link_directories(nfetch PRIVATE
    ${CHAFA_LIBRARY_DIRS}
)

target_link_libraries(nfetch PRIVATE
    ${CHAFA_LIBRARIES}
)

# GLIB (required by chafa)
target_include_directories(nfetch PRIVATE
    ${GLIB_INCLUDE_DIRS}
)
target_link_libraries(nfetch PRIVATE
    ${GLIB_LIBRARIES}
)