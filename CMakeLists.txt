cmake_minimum_required(VERSION 3.24)
project(STLHelper CXX)
set(CMAKE_CXX_STANDARD 17)

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

add_library(STLHelper SHARED
        STLHelper.cpp
        Exporter/common.h
        Exporter/types.h
        Exporter/utility.h
        )

target_include_directories(STLHelper PRIVATE
        Exporter
        ${CMAKE_CURRENT_LIST_DIR}
        ${FUSION_360_CPP_INCLUDE_DIR}
        )

target_link_libraries(STLHelper ${CORE_LIBRARY} ${FUSION_LIBRARY})
target_compile_features(STLHelper PRIVATE cxx_std_17)

# Zip File
install(DIRECTORY STLHelper.bundle DESTINATION .)
if (APPLE)
    install(TARGETS STLHelper LIBRARY DESTINATION STLHelper.bundle/Contents)
else()
    install(TARGETS STLHelper RUNTIME DESTINATION STLHelper.bundle/Contents)
endif()
set(CPACK_BINARY_NSIS OFF)
set(CPACK_BINARY_ZIP ON)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)