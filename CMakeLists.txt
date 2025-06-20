cmake_minimum_required(VERSION 3.11)
project(Graphark)

include(FetchContent)

find_package(OpenGL REQUIRED)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG 7b6aead9fb88b3623e3b3725ebb42670cbe4c579 # 3.4.0
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glew
  GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
  GIT_TAG f456deace7b408655109aaeff71421ef2d3858c6 # 2.2.0
)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e # refs/tags/1.0.1
)

FetchContent_Declare(
  expected
  GIT_REPOSITORY https://github.com/TartanLlama/expected
  GIT_TAG 292eff8bd8ee230a7df1d6a1c00c4ea0eb2f0362 # v1.1.0
)

FetchContent_Declare(
  cli11_proj
  QUIET
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.3.2
)

FetchContent_MakeAvailable(glfw glew glm expected cli11_proj)

set(graphark_lib "${PROJECT_NAME}_lib")
add_library(${graphark_lib}
  lib/graphark/src/function_evaluator.cpp
  lib/graphark/src/drawable_elements.cpp
)
target_include_directories(${graphark_lib} PUBLIC
  ${CMAKE_SOURCE_DIR}/lib/graphark/include
  ${CMAKE_SOURCE_DIR}/third_party/exprtk/include)
target_link_libraries(${graphark_lib} libglew_static)
set_target_properties(${graphark_lib} PROPERTIES CXX_STANDARD 23)
set_target_properties(${graphark_lib} PROPERTIES CXX_STANDARD_REQUIRED ON)

if(MSVC)
  message(STATUS "Adding /bigobj compiler flag for MSVC.")
  target_compile_options(${graphark_lib} PUBLIC /bigobj)
endif()

set(graphark "${PROJECT_NAME}")
add_executable(${graphark} src/main.cpp)
target_link_libraries(${graphark} ${graphark_lib} glfw libglew_static glm::glm expected CLI11::CLI11)
target_include_directories(${graphark} PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_target_properties(${graphark} PROPERTIES CXX_STANDARD 23)
set_target_properties(${graphark} PROPERTIES CXX_STANDARD_REQUIRED ON)
