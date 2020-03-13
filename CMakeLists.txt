cmake_minimum_required(VERSION 3.12)

project(qvina VERSION 1.0.0 LANGUAGES CXX)

# Load CMAKE configuration from environment variables
set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Boost library
set(BOOST_VERSION "1.71.0")
find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS system)
find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS serialization)
find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS filesystem)
find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS program_options)
find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS thread)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_ALL_DYN_LINK ON)

# Set the build type
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE "Release"
       CACHE STRING "Build configuration 'Release' or 'Debug'."
       FORCE )
endif()

# Install lib and binaries to ./bin
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/vina)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/qvina2)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/qvinaw)
