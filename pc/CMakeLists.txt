cmake_minimum_required(VERSION 3.20)
project(AnonymousMessagePC)

set(CMAKE_CXX_STANDARD 17)

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui Network Concurrent Sql PrintSupport)

# Find required libraries
find_package(OpenSSL REQUIRED)
find_package(Tor REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Executable
add_executable(AnonymousMessagePC ${SOURCES})

# Link libraries
target_link_libraries(AnonymousMessagePC 
    Qt5::Core 
    Qt5::Widgets 
    Qt5::Gui 
    Qt5::Network 
    Qt5::Concurrent 
    Qt5::Sql
    Qt5::PrintSupport
    OpenSSL::SSL 
    OpenSSL::Crypto
    tor
    pthread
)

# Platform-specific settings
if(WIN32)
    set_target_properties(AnonymousMessagePC PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(APPLE)
    set_target_properties(AnonymousMessagePC PROPERTIES MACOSX_BUNDLE TRUE)
    set_target_properties(AnonymousMessagePC PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER com.anonymousemessage.pc)
    set_target_properties(AnonymousMessagePC PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME AnonymousMessage)
endif()