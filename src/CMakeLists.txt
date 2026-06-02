cmake_minimum_required(VERSION 3.10)
project(ZygiskLoaderBypass CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download and build Dobby automatically during compilation
include(FetchContent)
FetchContent_Declare(
  Dobby
  GIT_REPOSITORY https://github.com/jmpews/Dobby.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(Dobby)

add_library(zygisk_loader_bypass SHARED main.cpp)
target_link_libraries(zygisk_loader_bypass log Dobby)
