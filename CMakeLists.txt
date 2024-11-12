cmake_minimum_required(VERSION 3.10)

project(AttoTest)

set(CMAKE_CONFIGURATION_TYPES Debug;Release)
set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

file(GLOB_RECURSE SOURCE_FILES_MAIN RELATIVE ${CMAKE_BINARY_DIR}/.. 
    "src/socket/*"
    "src/utils/*"
    "src/logic/*"
    "src/containers/*"
    "src/main.cpp")
file(GLOB_RECURSE SOURCE_FILES_UDP RELATIVE ${CMAKE_BINARY_DIR}/.. 
    "src/socket/*"
    "src/utils/*"
    "src/logic/message.*"
    "src/apps/udpSender.cpp")
file(GLOB_RECURSE SOURCE_FILES_TCP RELATIVE ${CMAKE_BINARY_DIR}/..
    "src/socket/*"
    "src/utils/*"
    "src/logic/message.*"
    "src/apps/tcpListener.cpp")

source_group(TREE ${CMAKE_BINARY_DIR}/..)

add_executable(AttoTest ${SOURCE_FILES_MAIN})
add_executable(AttoUDPSend ${SOURCE_FILES_UDP})
add_executable(AttoTCPListen ${SOURCE_FILES_TCP})

if(WIN32)
    target_link_libraries(AttoTest PRIVATE
        Ws2_32.lib
        Mswsock.lib
    )

    target_link_libraries(AttoUDPSend PRIVATE
        Ws2_32.lib
        Mswsock.lib
    )

    target_link_libraries(AttoTCPListen PRIVATE
        Ws2_32.lib
        Mswsock.lib
    )
endif()