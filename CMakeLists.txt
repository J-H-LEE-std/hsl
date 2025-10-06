cmake_minimum_required(VERSION 3.27)
project(hsl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 소스 코드 모음
file(GLOB_RECURSE PROJ_SRC "src/*.cpp" "src/*.h")

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(hsl ${PROJ_SRC}
        src/interpreter/func.h
        src/interpreter/func.cpp)

if (WIN32)
    set_target_properties(hsl PROPERTIES OUTPUT_NAME "hsl")
elseif (APPLE)
    set_target_properties(hsl PROPERTIES OUTPUT_NAME "hsl-macos")
elseif (UNIX)
    set_target_properties(hsl PROPERTIES OUTPUT_NAME "hsl-linux")
endif()

include(FetchContent)
FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.5.0
)

FetchContent_MakeAvailable(CLI11)

target_link_libraries(hsl PRIVATE CLI11::CLI11)