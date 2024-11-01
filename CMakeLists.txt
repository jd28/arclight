cmake_minimum_required(VERSION 3.21)
project(arclight VERSION 1.0.0 LANGUAGES CXX C)
set (CMAKE_CXX_STANDARD 20)

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

include(cmake/PreventInSourceBuilds.cmake)
include(cmake/ProjectSettings.cmake)

# Generate compile_commands.json to make it easier to work with clang based tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# AppleClang changes visibility to hidden
if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-fvisibility=hidden)
endif()

if(MSVC)
elseif(UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-sized-deallocation")
endif()

add_subdirectory(external)

# Don't set before external
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
add_subdirectory(src)
