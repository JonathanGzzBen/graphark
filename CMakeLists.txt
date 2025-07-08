cmake_minimum_required(VERSION 3.11)
project(Graphark)

include(FetchContent)

find_package(OpenGL REQUIRED)

# ---- glfw ----
option(GRAPHARK_FETCH_GLFW "Use FetchContent for glfw (overrides vcpkg)" OFF)
if (GRAPHARK_FETCH_GLFW)
    message(STATUS "Using FetchContent for glfw")
    include(FetchContent)
    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw
            GIT_TAG 7b6aead9fb88b3623e3b3725ebb42670cbe4c579 # 3.4.0
            FIND_PACKAGE_ARGS
    )
    FetchContent_MakeAvailable(glfw)
else ()
    message(STATUS "Using find_package(glfw3)")
    find_package(glfw3 REQUIRED)
endif ()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# ---- glew ----
option(GRAPHARK_FETCH_GLEW "Use FetchContent for glew (overrides vcpkg)" OFF)
if (GRAPHARK_FETCH_GLEW)
    message(STATUS "Using FetchContent for glew")
    include(FetchContent)
    FetchContent_Declare(
            glew
            GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
            GIT_TAG f456deace7b408655109aaeff71421ef2d3858c6 # 2.2.0
            FIND_PACKAGE_ARGS
    )
    FetchContent_MakeAvailable(glew)
else ()
    message(STATUS "Using find_package(glew)")
    find_package(glew REQUIRED)
endif ()

# ---- glm ----
option(GRAPHARK_FETCH_GLM "Use FetchContent for glm (overrides vcpkg)" OFF)
if (GRAPHARK_FETCH_GLM)
    message(STATUS "Using FetchContent for glm")
    include(FetchContent)
    FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm
            GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e # refs/tags/1.0.1
    )
    FetchContent_MakeAvailable(glm)
else ()
    message(STATUS "Using find_package(glm)")
    find_package(glm REQUIRED)
endif ()

# ---- tl-expected ----
option(GRAPHARK_FETCH_TL_EXPECTED "Use FetchContent for tl-expected (overrides vcpkg)" OFF)
if (GRAPHARK_FETCH_TL_EXPECTED)
    message(STATUS "Using FetchContent for tl-expected")
    include(FetchContent)
    FetchContent_Declare(
            tl-expected
            GIT_REPOSITORY https://github.com/TartanLlama/expected
            GIT_TAG 292eff8bd8ee230a7df1d6a1c00c4ea0eb2f0362 # v1.1.0
    )
    FetchContent_MakeAvailable(tl-expected)
else ()
    message(STATUS "Using find_package(tl-expected)")
    find_package(tl-expected REQUIRED)
endif ()

# ---- CLI11 ----
option(GRAPHARK_FETCH_CLI11 "Use FetchContent for CLI11 (overrides vcpkg)" OFF)
if (GRAPHARK_FETCH_CLI11)
    message(STATUS "Using FetchContent for CLI11")
    include(FetchContent)
    FetchContent_Declare(
            CLI11
            GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
            GIT_TAG v2.3.2
    )
    FetchContent_MakeAvailable(CLI11)
else ()
    message(STATUS "Using find_package(CLI11)")
    find_package(CLI11 REQUIRED)
endif ()

# ---- Exprtk ----
add_library(exprtk INTERFACE)
option(GRAPHARK_FETCH_EXPRTK "Download exprtk.hpp (overrides vcpkg)" OFF)
if (GRAPHARK_FETCH_EXPRTK)
    set(EXPRTK_URL https://raw.githubusercontent.com/ArashPartow/exprtk/master/exprtk.hpp)
    set(EXPRTK_DEST ${CMAKE_CURRENT_BINARY_DIR}/exprtk.hpp)

    if (NOT EXISTS ${EXPRTK_DEST})
        file(DOWNLOAD ${EXPRTK_URL} ${EXPRTK_DEST} SHOW_PROGRESS)
    endif ()

    target_include_directories(exprtk INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
else ()
    find_path(EXPRTK_INCLUDE_DIRS "exprtk.hpp" REQUIRED)
endif ()

add_library(graphark_lib
        lib/src/function_evaluator.cpp
        lib/src/drawable_elements.cpp
)
target_include_directories(graphark_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/include)
target_link_libraries(graphark_lib GLEW::GLEW exprtk)
set_target_properties(graphark_lib PROPERTIES CXX_STANDARD 23)
set_target_properties(graphark_lib PROPERTIES CXX_STANDARD_REQUIRED ON)

if (MSVC)
    message(STATUS "Adding /bigobj compiler flag for MSVC.")
    target_compile_options(graphark_lib PUBLIC /bigobj)
endif ()

add_executable(graphark src/main.cpp)

target_link_libraries(graphark graphark_lib glfw GLEW::GLEW glm::glm tl::expected CLI11::CLI11)
target_include_directories(graphark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(graphark PROPERTIES CXX_STANDARD 23)
set_target_properties(graphark PROPERTIES CXX_STANDARD_REQUIRED ON)
