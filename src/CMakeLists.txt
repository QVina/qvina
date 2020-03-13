cmake_minimum_required(VERSION 3.12)

# Source files
file(GLOB HEADERS_FILES "include/*.h")
file(GLOB SOURCE_FILES "lib/*.cpp")
set(VINA_FILE "vina/vina.cpp")
set(VINA_SPLIT_FILE "vina_split/vina_split.cpp")

# libqvina
add_library(qvina STATIC ${SOURCE_FILES})

target_include_directories(qvina PUBLIC
                                 ${Boost_INCLUDE_DIRS}
                                 ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_link_libraries(qvina PUBLIC
                            Boost::boost
                            Boost::system
                            Boost::serialization
                            Boost::filesystem
                            Boost::program_options
                            Boost::thread)

# vina
add_executable(vina ${VINA_FILE} ${HEADERS_FILES})
target_link_libraries(vina qvina)

# vina_split
add_executable(vina_split ${VINA_FILE} ${HEADERS_FILES})
target_link_libraries(vina_split qvina)
