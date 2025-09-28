//
// Created by IDKTHIS on 09.09.2025.
//

#include "FileSystem.h"

#include <filesystem>

#include "Core/Log/Logger.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(PLATFORM_LINUX)
#include <unistd.h>
#endif

using namespace Engine;

std::string FileSystem::GetWorkingDirectory()
{
    std::filesystem::path path;

#if defined(PLATFORM_WINDOWS)
    char buffer[MAX_PATH];
    DWORD size = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    path = std::filesystem::path(buffer);

#elif defined(PLATFORM_LINUX)
    char buffer[1024] = {0};
    ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
    if (count != -1) buffer[count] = '\0';
    path = std::filesystem::path(buffer);

#else
#error "Unsupported platform"
#endif

    return (path.parent_path() / "").string();
}

bool FileSystem::CreateFileIfNotExists(const std::string& filePath, const std::string& defaultContent)
{
    std::filesystem::path path(filePath);

    try {
        if (!std::filesystem::exists(path.parent_path())) {
            std::filesystem::create_directories(path.parent_path());
        }

        if (!std::filesystem::exists(path)) {
            std::ofstream file(path);
            if (!file.is_open()) {
                return false;
            }
            if (!defaultContent.empty()) {
                file << defaultContent;
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("FileSystem", "Failed to create file {}: {}", filePath, e.what());
        return false;
    }
    return true;
}
