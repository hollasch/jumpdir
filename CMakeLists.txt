cmake_minimum_required(VERSION 3.2...3.27.8)

set (CMAKE_CXX_STANDARD 17)

project (jumpdir LANGUAGES CXX)

add_library (json INTERFACE)
target_include_directories (json INTERFACE src/json)

add_executable (jumpdir
    src/jumpdir.cpp
    src/ext/FileSystemProxy/FileSystemProxy.h
    src/ext/FileSystemProxy/FileSystemProxyWindows.h
    src/ext/FileSystemProxy/FileSystemProxyWindows.cpp
    src/ext/PathMatcher/PathMatcher.h
    src/ext/PathMatcher/PathMatcher.cpp
)

target_link_libraries (jumpdir PRIVATE json)

include_directories(src src/ext/PathMatcher src/ext/FileSystemProxy)
