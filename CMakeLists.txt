cmake_minimum_required (VERSION 3.20)

if(NOT DEFINED ENV{VCPKG_ROOT})
    if(EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/vcpkg")
        set(ENV{VCPKG_ROOT} "${CMAKE_SOURCE_DIR}/thirdparty/vcpkg")
    else()
        message(FATAL_ERROR "VCPKG_ROOT is not defined and thirdparty/vcpkg not found!")
    endif()
endif()

set(FERNANDO_RECOMP_THIRDPARTY_ROOT ${CMAKE_SOURCE_DIR}/thirdparty)
set(FERNANDO_RECOMP_TOOLS_ROOT ${CMAKE_SOURCE_DIR}/tools)
set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS OFF)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project("GTA4Recomp-ALL")

if (APPLE)
    enable_language(OBJC OBJCXX)
endif()

if (CMAKE_OSX_ARCHITECTURES)
    set(FERNANDO_RECOMP_ARCHITECTURE ${CMAKE_OSX_ARCHITECTURES})
elseif(CMAKE_SYSTEM_PROCESSOR)
    set(FERNANDO_RECOMP_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
else()
    set(FERNANDO_RECOMP_ARCHITECTURE ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif()
string(TOLOWER "${FERNANDO_RECOMP_ARCHITECTURE}" FERNANDO_RECOMP_ARCHITECTURE)
message(STATUS "Detected architecture: ${FERNANDO_RECOMP_ARCHITECTURE}")

if (FERNANDO_RECOMP_ARCHITECTURE STREQUAL "x86_64" OR FERNANDO_RECOMP_ARCHITECTURE STREQUAL "amd64")
    # Target Sandy Bridge for all projects
    add_compile_options(
        -march=sandybridge
    )
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Normally only defined by Visual Studio, added for consistency
    add_compile_definitions(_DEBUG)
endif()

add_subdirectory(${FERNANDO_RECOMP_THIRDPARTY_ROOT})
add_subdirectory(${FERNANDO_RECOMP_TOOLS_ROOT})

# Include sub-projects.
add_subdirectory("GTA4RecompLib")
add_subdirectory("GTA4Recomp")
