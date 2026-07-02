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
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    $<$<CXX_COMPILER_ID:MSVC>:/permissive->
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
    src/log/ExperimentLogger.cpp
    src/interpreter/evaluator.cpp
    src/interpreter/func.cpp
    src/interpreter/lexer.cpp
    src/interpreter/parser.cpp
    src/utils/printer.cpp
    src/utils/timing.cpp
)
set(HSL_CORE_HDR
    src/hs/io.h 
    src/hs/hsalgorithm.h
    src/hs/params.h    src/hs/runner.h
    src/log/ExperimentLogger.h
    src/interpreter/ast.h
    src/interpreter/evaluator.h
    src/interpreter/func.h
    src/interpreter/lexer.h
    src/interpreter/parser.h
    src/interpreter/token.h
    src/utils/printer.h
    src/utils/timing.h
)
add_library(hsl_core STATIC ${HSL_CORE_SRC} ${HSL_CORE_HDR})
target_include_directories(hsl_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hsl src/climain.cpp)
target_link_libraries(hsl PRIVATE hsl_core CLI11::CLI11)

if(UNIX AND NOT APPLE) 
    target_link_options(hsl PRIVATE -static-libgcc -static-libstdc++)
endif()

set(wxWidgets_USE_STATIC ON)
set(wxWidgets_USE_UNICODE ON)
set(wxBUILD_SHARED OFF CACHE BOOL "Build wxWidgets as static libs" FORCE)

if(WIN32)
    if(NOT wxWidgets_ROOT_DIR AND DEFINED ENV{WXWIN})
        file(TO_CMAKE_PATH "$ENV{WXWIN}" wxWidgets_ROOT_DIR)
    endif()

    if(wxWidgets_ROOT_DIR AND NOT wxWidgets_LIB_DIR)
        set(_wx_candidate_lib_dirs
            "${wxWidgets_ROOT_DIR}/lib/vc_x64_lib"
            "${wxWidgets_ROOT_DIR}/lib/vc_lib"
        )

        foreach(_wx_lib_dir IN LISTS _wx_candidate_lib_dirs)
            if(
                EXISTS "${_wx_lib_dir}/mswu/wx/setup.h"
                OR EXISTS "${_wx_lib_dir}/mswud/wx/setup.h"
                OR EXISTS "${_wx_lib_dir}/msw/wx/setup.h"
                OR EXISTS "${_wx_lib_dir}/mswd/wx/setup.h"
            )
                set(wxWidgets_LIB_DIR "${_wx_lib_dir}" CACHE PATH "wxWidgets library directory" FORCE)
                break()
            endif()
        endforeach()

        unset(_wx_candidate_lib_dirs)
        unset(_wx_lib_dir)
    endif()
endif()

set(_wx_components core base aui html adv net xml)

if(WIN32)
    find_package(wxWidgets MODULE QUIET COMPONENTS ${_wx_components})
else()
    find_package(wxWidgets QUIET COMPONENTS ${_wx_components})
endif()

unset(_wx_components)

if(wxWidgets_FOUND)
    message(STATUS "Found wxWidgets: ${wxWidgets_LIBRARIES}")
    if(wxWidgets_USE_FILE)
        include(${wxWidgets_USE_FILE})
    endif()

    set(HSL_GUI_SRC
        src/main.cpp
        src/gui/mainwindow.cpp
        src/gui/parmwindow.cpp
        src/gui/jsonlogwindow.cpp
        src/gui/bridge.cpp
        src/gui/io_gui.cpp
    )
    set(HSL_GUI_HDR
        src/gui/mainwindow.h
        src/gui/parmwindow.h
        src/gui/jsonlogwindow.h
        src/gui/bridge.h
        src/gui/paramstruct.h
        src/gui/jsonlogsettings.h
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
    message(WARNING
        "wxWidgets not found. GUI target 'hsl_gui' will be skipped. "
        "On Windows, use a standard wxWidgets MSW build layout and set "
        "wxWidgets_ROOT_DIR (or environment variable WXWIN) so CMake can find "
        "include/wx and lib/vc_x64_lib/mswu/wx/setup.h."
    )
endif()
