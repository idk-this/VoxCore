cmake_minimum_required(VERSION 3.16)
project(VoxCore VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ENGINE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(ENGINE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(ENGINE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(ENGINE_INFO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Core/EngineInfo.h")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/build_number_trigger
        COMMAND ${CMAKE_COMMAND}
        -D ENGINE_INFO_FILE=${ENGINE_INFO_FILE}
        -D PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        -D PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        -D PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/update_build_number.cmake
        COMMENT "Updating build number in EngineInfo.h"
        VERBATIM
)

function(get_build_number)
    if(EXISTS "${ENGINE_INFO_FILE}")
        file(READ "${ENGINE_INFO_FILE}" ENGINE_INFO_CONTENT)
        string(REGEX MATCH "#define ENGINE_BUILD_NUMBER ([0-9]+)" _match "${ENGINE_INFO_CONTENT}")
        if(_match)
            string(REGEX REPLACE ".*#define ENGINE_BUILD_NUMBER ([0-9]+).*" "\\1" ENGINE_BUILD_NUMBER "${ENGINE_INFO_CONTENT}")
        else()
            set(ENGINE_BUILD_NUMBER 0 PARENT_SCOPE)
        endif()
    else()
        set(ENGINE_BUILD_NUMBER 0 PARENT_SCOPE)
    endif()
    set(ENGINE_BUILD_NUMBER ${ENGINE_BUILD_NUMBER} PARENT_SCOPE)
endfunction()

get_build_number()

string(TIMESTAMP ENGINE_BUILD_DATE "%Y-%m-%d %H:%M:%S")

if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(UNIX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

set(RESOURCES_SRC "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
set(RESOURCES_DST "${CMAKE_BINARY_DIR}/$<CONFIG>/Content")

set(LIBS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/runtime_libs")
set(LIBS_DST "${CMAKE_BINARY_DIR}/$<CONFIG>/Engine")

add_subdirectory(libs/googletest)
add_subdirectory(libs/pugixml)
add_subdirectory(libs/json)

set(SDL_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(libs/SDL)

find_package(Vulkan REQUIRED COMPONENTS glslang SPIRV-Tools)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE libs/stb)

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
        ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan)

file(GLOB_RECURSE ENGINE_SRC
        Application/*.cpp
        Core/*.cpp
        Core/**/*.cpp
        Platform/*.cpp
        Platform/**/*.cpp
)
file(GLOB_RECURSE ENGINE_HDR
        Application/*.h
        Core/*.h
        Core/**/*.h
        Platform/*.h
        Platform/**/*.h
)

add_library(VoxCore SHARED ${ENGINE_SRC} ${ENGINE_HDR} ${CMAKE_CURRENT_BINARY_DIR}/build_number_trigger)

target_include_directories(VoxCore PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        libs/glm
        libs/stb
        libs/SDL/include
        ${Vulkan_INCLUDE_DIRS}
        "$ENV{VULKAN_SDK}/Include"
        libs/pugixml/src
)

target_link_libraries(VoxCore PRIVATE
        imgui
        SDL3-static
        Vulkan::Vulkan
        pugixml
        nlohmann_json::nlohmann_json
)

if(WIN32)
    target_link_libraries(VoxCore PRIVATE "$ENV{VULKAN_SDK}/Lib/shaderc_shared.lib")
elseif(UNIX)
    find_library(SHADERC_LIB shaderc_shared REQUIRED)
    target_link_libraries(VoxCore PRIVATE ${SHADERC_LIB})
endif()

target_compile_definitions(VoxCore PRIVATE VOXCORE_BUILD_DLL)

set_target_properties(VoxCore PROPERTIES
        PREFIX ""
        OUTPUT_NAME "VoxCore.Engine"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/Engine"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/Engine"
)

add_custom_command(TARGET VoxCore POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${RESOURCES_DST}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${RESOURCES_SRC}" "${RESOURCES_DST}"
        COMMENT "Copying resources to ${RESOURCES_DST}"
)
if (EXISTS "${LIBS_SRC}" AND IS_DIRECTORY "${LIBS_SRC}")
    file(GLOB LIB_FILES "${LIBS_SRC}/*")
    if (LIB_FILES)
        add_custom_command(TARGET VoxCore POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${LIBS_DST}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIBS_SRC}" "${LIBS_DST}"
                COMMENT "Copying libs to ${LIBS_DST}"
        )
    endif()
endif()

add_executable(VoxCoreTests Tests/Basics/SDL_Window_Tests.cpp)
target_link_libraries(VoxCoreTests PRIVATE VoxCore gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(VoxCoreTests)