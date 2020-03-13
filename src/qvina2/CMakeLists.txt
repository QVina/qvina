cmake_minimum_required(VERSION 3.12)

# Source files
file(GLOB HEADERS_FILES "include/*.h")
file(GLOB SOURCE_FILES "lib/*.cpp")
set(VINA_FILE "main.cpp")
set(VINA_SPLIT_FILE "split.cpp")

# libqvina
add_library(libqvina2 STATIC ${SOURCE_FILES})
target_include_directories(libqvina2 PUBLIC
                                 ${Boost_INCLUDE_DIRS}
                                 ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_link_libraries(libqvina2 PUBLIC
                            Boost::boost
                            Boost::system
                            Boost::serialization
                            Boost::filesystem
                            Boost::program_options
                            Boost::thread)

# vina
add_executable(qvina2 ${VINA_FILE} ${HEADERS_FILES})
target_link_libraries(qvina2 libqvina2)

# vina_split
add_executable(qvina2_split ${VINA_FILE} ${HEADERS_FILES})
target_link_libraries(qvina2_split libqvina2)

install(TARGETS qvina2
        EXPORT qvina2
        RUNTIME DESTINATION bin
        )

install(TARGETS qvina2_split
        EXPORT qvina2_split
        RUNTIME DESTINATION bin
        )
