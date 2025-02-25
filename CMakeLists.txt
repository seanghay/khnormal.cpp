cmake_minimum_required(VERSION 3.18)

project(khnormal)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cc khnormal.cc)

add_subdirectory(utfcpp)
add_subdirectory(re2)

target_link_libraries(${PROJECT_NAME}
  PRIVATE utf8cpp
  PRIVATE re2::re2
)


