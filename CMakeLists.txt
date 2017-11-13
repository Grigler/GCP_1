cmake_minimum_required(VERSION 2.6)
project(GCP_1)

include_directories(
  ${PROJECT_SOURCE_DIR}/src
)

add_executable(Brute_Force
  src/Brute_Force/tiny_obj_loader.cc
  src/Brute_Force/lodepng.cpp

  src/Brute_Force/main.cpp

  src/Brute_Force/Ray.cpp
)