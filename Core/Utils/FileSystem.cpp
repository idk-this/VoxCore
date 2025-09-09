//
// Created by IDKTHIS on 09.09.2025.
//

#include "FileSystem.h"

#include <filesystem>

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(PLATFORM_LINUX)
#include <unistd.h>
#endif

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
