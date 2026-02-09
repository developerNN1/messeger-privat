cmake_minimum_required(VERSION 3.16)
project(AnonymousMessagePC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui Network Concurrent Sql)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OPENSSL_INCLUDE_DIRS}
)

# Define source files
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/UserManager.cpp
    src/network/TorManager.cpp
    src/network/MessageHandler.cpp
    src/ui/MainWindow.cpp
    src/ui/LoginWindow.cpp
    src/ui/ChatWindow.cpp
    src/ui/CallWindow.cpp
    src/utils/EncryptionUtils.cpp
    src/utils/ValidationUtils.cpp
)

set(HEADERS
    src/core/Application.h
    src/core/UserManager.h
    src/network/TorManager.h
    src/network/MessageHandler.h
    src/ui/MainWindow.h
    src/ui/LoginWindow.h
    src/ui/ChatWindow.h
    src/ui/CallWindow.h
    src/utils/EncryptionUtils.h
    src/utils/ValidationUtils.h
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Network
    Qt5::Concurrent
    Qt5::Sql
    ${OPENSSL_LIBRARIES}
    tor
    ssl
    crypto
)

# Platform-specific configurations
if(WIN32)
    # Windows-specific settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
elseif(APPLE)
    # macOS-specific settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.anonymousmessage.pc
        MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
    )
endif()