cmake_minimum_required(VERSION 3.24)
project(STLExport CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

find_package(Python)

set(POSSIBLE_FUSION_360_API_DIRS
        "${CMAKE_SOURCE_DIR}/fusion_360_api"
        "$ENV{APPDATA}/Autodesk/Autodesk Fusion 360/API/CPP"
        "$ENV{HOME}/Library/Application Support/Autodesk/Autodesk Fusion 360/API/CPP"
)

find_path(FUSION_360_CPP_INCLUDE_DIR
        NAMES Fusion/FusionAll.h Core/CoreAll.h
        REQUIRED
        HINTS ${POSSIBLE_FUSION_360_API_DIRS}
        PATH_SUFFIXES include
)


find_library(CORE_LIBRARY
        core.lib core.dylib
        REQUIRED
        HINTS ${POSSIBLE_FUSION_360_API_DIRS}
        PATH_SUFFIXES lib)

find_library(FUSION_LIBRARY
        fusion.lib fusion.dylib
        REQUIRED
        HINTS ${POSSIBLE_FUSION_360_API_DIRS}
        PATH_SUFFIXES lib)


IF(MSVC)
    SET(CMAKE_CXX_FLAGS "/EHsc") # Enable exception unwind semantics in the compiler
ENDIF(MSVC)

set (_src

        ExporterUI.cpp
        ExporterUI.h
        STLExport.cpp
        ExporterPlatform.h
)

add_library(STLExport SHARED ${_src})

set_target_properties(STLExport PROPERTIES PREFIX "")

target_include_directories(STLExport PRIVATE
        Exporter
        ${CMAKE_CURRENT_LIST_DIR}
        ${FUSION_360_CPP_INCLUDE_DIR}
        )

target_link_libraries(STLExport ${CORE_LIBRARY} ${FUSION_LIBRARY})
target_compile_features(STLExport PRIVATE cxx_std_17)

# Zip File
install(DIRECTORY STLExport.bundle DESTINATION .)
if (APPLE)
    install(TARGETS STLExport LIBRARY DESTINATION STLExport.bundle/Contents)
else()
    install(TARGETS STLExport RUNTIME DESTINATION STLExport.bundle/Contents)
endif()
set(CPACK_BINARY_NSIS OFF)
set(CPACK_BINARY_ZIP ON)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)