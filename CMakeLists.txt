cmake_minimum_required(VERSION 3.27)
project(hsl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_compile_options(
    $<$<C_COMPILER_ID:MSVC>:/utf-8>
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    $<$<CXX_COMPILER_ID:MSVC>:/permissive->
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-finput-charset=UTF-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-finput-charset=UTF-8>
)

include(FetchContent)
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.5.0
)
FetchContent_MakeAvailable(CLI11)

set(HSL_CORE_SRC
    src/hs/io.cpp 
    src/hs/hsalgorithm.cpp
    src/hs/runner.cpp
    src/interpreter/evaluator.cpp
    src/interpreter/func.cpp
    src/interpreter/lexer.cpp
    src/interpreter/parser.cpp
    src/utils/printer.cpp
)
set(HSL_CORE_HDR
    src/hs/io.h 
    src/hs/hsalgorithm.h
    src/hs/params.h    src/hs/runner.h
    src/interpreter/ast.h
    src/interpreter/evaluator.h
    src/interpreter/func.h
    src/interpreter/lexer.h
    src/interpreter/parser.h
    src/interpreter/token.h
    src/utils/printer.h
)
add_library(hsl_core STATIC ${HSL_CORE_SRC} ${HSL_CORE_HDR})
target_include_directories(hsl_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hsl src/climain.cpp)
target_link_libraries(hsl PRIVATE hsl_core CLI11::CLI11)

if(UNIX AND NOT APPLE) 
    target_link_options(hsl PRIVATE -static-libgcc -static-libstdc++)
endif()

set(wxWidgets_USE_STATIC ON)
set(wxBUILD_SHARED OFF CACHE BOOL "Build wxWidgets as static libs" FORCE)

find_package(wxWidgets COMPONENTS core base aui html adv net xml REQUIRED)
if(wxWidgets_FOUND)
    message(STATUS "Found wxWidgets: ${wxWidgets_LIBRARIES}")
    include(${wxWidgets_USE_FILE})

    set(HSL_GUI_SRC
        src/main.cpp
        src/gui/mainwindow.cpp
        src/gui/parmwindow.cpp
        src/gui/bridge.cpp
        src/gui/io_gui.cpp
    )
    set(HSL_GUI_HDR
        src/gui/mainwindow.h
        src/gui/parmwindow.h
        src/gui/bridge.h
        src/gui/paramstruct.h
    )

    if(WIN32)
        add_executable(hsl_gui WIN32 ${HSL_GUI_SRC} ${HSL_GUI_HDR})
        add_definitions(-DwxUSE_NO_MANIFEST=1)
    elseif(APPLE)
        add_executable(hsl_gui MACOSX_BUNDLE ${HSL_GUI_SRC} ${HSL_GUI_HDR})
    else()
        add_executable(hsl_gui ${HSL_GUI_SRC} ${HSL_GUI_HDR})
    endif()

     target_link_libraries(hsl_gui PRIVATE
        hsl_core
        ${wxWidgets_LIBRARIES}
    )

    target_include_directories(hsl_gui PRIVATE
        ${wxWidgets_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
        ${CMAKE_CURRENT_SOURCE_DIR}/src/hs
    )

     target_compile_definitions(hsl_gui PRIVATE HSL_GUI)

    if(UNIX AND NOT APPLE) 
        target_link_options(hsl_gui PRIVATE -static-libgcc -static-libstdc++)
    endif()

else()
    message(WARNING "wxWidgets not found. Skipping 'hsl_gui' target.")
endif()
