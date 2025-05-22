cmake_minimum_required(VERSION 3.16)

project(RtMidi LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: Only needed if using Qt features (signals/slots, QObject, etc.)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# ✅ Define the executable with all your source files
add_executable(RtMidi
    RtMidiIn.cpp         # <-- Your test file (can be renamed to main.cpp)
    RtMidi.cpp
    RtMidi.h
)

# ✅ Add include path if needed
target_include_directories(RtMidi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ✅ Define __WINDOWS_MM__ to enable real MIDI on Windows
if(WIN32)
    target_compile_definitions(RtMidi PRIVATE __WINDOWS_MM__)
    target_link_libraries(RtMidi PRIVATE winmm)
endif()

# ✅ (Optional) Link QtCore if you use it in your code
target_link_libraries(RtMidi PRIVATE Qt${QT_VERSION_MAJOR}::Core)

# Install rules (can be left as-is)
include(GNUInstallDirs)
install(TARGETS RtMidi
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
