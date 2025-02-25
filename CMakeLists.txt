cmake_minimum_required(VERSION 3.18)

project(khnormal)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

add_executable(${PROJECT_NAME} main.cc khnormal.cc)
add_subdirectory(utfcpp)

set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_ENABLE_INSTALL ON)
add_subdirectory(abseil-cpp)

add_subdirectory(re2)

target_link_libraries(${PROJECT_NAME}
  PRIVATE utf8cpp
  PRIVATE re2::re2
)


