cmake_minimum_required(VERSION 3.16)
project(VoxCore)

set(CMAKE_CXX_STANDARD 23)

enable_testing()
if (WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(UNIX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

# === GoogleTest ===
add_subdirectory(libs/googletest)

# === SDL3 ===
set(SDL_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(libs/SDL)

# === Vulkan ===
find_package(Vulkan REQUIRED COMPONENTS glslang SPIRV-Tools)

# === GLM ===
set(GLM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/glm")

# === ImGui ===
file(GLOB IMGUI_SOURCES
        libs/imgui/*.cpp
        libs/imgui/backends/imgui_impl_sdl3.cpp
        libs/imgui/backends/imgui_impl_vulkan.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
        libs/imgui
        libs/imgui/backends
        libs/SDL/include
        ${GLM_INCLUDE_DIR}
        ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan)

# === VoxCore Engine (DLL) ===
file(GLOB_RECURSE ENGINE_SRC
        Application/*.cpp
        Core/**/*.cpp
        Platform/**/*.cpp
        Platform/**/**/*.cpp
)
file(GLOB_RECURSE ENGINE_HDR
        Application/*.h
        Core/**/*.h
        Platform/**/*.h
        Platform/**/**/*.h
)

add_library(VoxCore SHARED ${ENGINE_SRC} ${ENGINE_HDR})
target_include_directories(VoxCore PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLM_INCLUDE_DIR}
        libs/SDL/include
        ${Vulkan_INCLUDE_DIRS}
        "$ENV{VULKAN_SDK}/Include"
)

if (WIN32)
    target_link_libraries(VoxCore
            imgui
            SDL3-static
            Vulkan::Vulkan
            "$ENV{VULKAN_SDK}/Lib/shaderc_shared.lib"
    )
elseif(UNIX)
    find_library(SHADERC_LIB shaderc_shared REQUIRED)
    target_link_libraries(VoxCore
            imgui
            SDL3-static
            Vulkan::Vulkan
            ${SHADERC_LIB}
    )
endif()

set_target_properties(VoxCore PROPERTIES
        PREFIX ""
        OUTPUT_NAME "VoxCore"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Engine/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Engine/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Engine/bin
)


# === Unit Tests ===
add_executable(VoxCoreTests
        Tests/Basics/SDL_Window_Tests.cpp
)
target_include_directories(VoxCoreTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        libs/SDL/include
        ${GLM_INCLUDE_DIR}
        ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(VoxCoreTests
        VoxCore
        gtest
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(VoxCoreTests)
