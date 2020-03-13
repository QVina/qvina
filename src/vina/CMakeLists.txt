cmake_minimum_required(VERSION 3.12)

# Source files
file(GLOB HEADERS_FILES "include/*.h")
file(GLOB SOURCE_FILES "lib/*.cpp")
set(VINA_FILE "main.cpp")
set(VINA_SPLIT_FILE "split.cpp")

# libqvina
add_library(libvina STATIC ${SOURCE_FILES})
target_include_directories(libvina PUBLIC
                                 ${Boost_INCLUDE_DIRS}
                                 ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_link_libraries(libvina PUBLIC
                            Boost::boost
                            Boost::system
                            Boost::serialization
                            Boost::filesystem
                            Boost::program_options
                            Boost::thread)

# vina
add_executable(vina ${VINA_FILE} ${HEADERS_FILES})
target_link_libraries(vina libvina)

# vina_split
add_executable(vina_split ${VINA_FILE} ${HEADERS_FILES})
target_link_libraries(vina_split libvina)

install(TARGETS vina
        EXPORT vina
        RUNTIME DESTINATION bin
        )

install(TARGETS vina_split
        EXPORT vina_split
        RUNTIME DESTINATION bin
        )
