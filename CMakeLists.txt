cmake_minimum_required(VERSION 2.6)
project(GCP_1)

include_directories(
  ${PROJECT_SOURCE_DIR}/src
)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_executable(Brute_Force
  src/Brute_Force/tiny_obj_loader.cc
  src/Brute_Force/lodepng.cpp

  src/Brute_Force/main.cpp

  src/Brute_Force/Ray.cpp
)