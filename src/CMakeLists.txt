cmake_minimum_required(VERSION 3.10)
project(ZygiskLoaderBypass CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download and build Dobby automatically
include(FetchContent)
FetchContent_Declare(
  dobby
  GIT_REPOSITORY https://github.com/jmpews/Dobby.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(dobby)

add_library(zygisk_loader_bypass SHARED main.cpp)

# Fix Dobby include error
target_include_directories(zygisk_loader_bypass PRIVATE ${dobby_SOURCE_DIR}/include)
target_link_libraries(zygisk_loader_bypass log dobby)
