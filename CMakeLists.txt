cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

project(Vulkan)

find_package(Vulkan REQUIRED)

if (${Vulkan_VERSION} VERSION_LESS "1.1.000" )
    message( FATAL_ERROR "Minimum required Vulkan version for C++ modules is 1.1.000. "
            "Found ${Vulkan_VERSION}."
    )
endif()

add_subdirectory(Vendor/glfw)
add_subdirectory(Vendor/glad)
add_subdirectory(Vendor/glm)
add_subdirectory(Vendor/stb_image)
add_subdirectory(Vendor/imgui)
add_subdirectory(Vendor/spdlog)

include(cmake/configure_target.cmake)

add_subdirectory(Nest)

add_subdirectory(Examples/Sandbox)
add_subdirectory(Examples/Triangle)