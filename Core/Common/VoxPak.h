//
// Created by IDKTHIS on 09.09.2025.
//

#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

#include "Core/Export.h"

struct FileHeader {
    char magic[8];
    uint32_t fileCount;
};
class VOXCORE_API VoxPak {
public:
    bool Open(const std::string& pakFile);
    [[nodiscard]] std::vector<std::string> ListFiles() const;
    std::vector<uint8_t> ReadFile(const std::string& path) const;
    std::vector<std::filesystem::path> pakModOverride;

    std::vector<uint8_t> ReadFileWithOverride(const std::string& path) const;
    std::string ReadFileWithOverrideString(const std::string& path) const;

private:
    struct Entry {
        std::string path;
        uint64_t offset;
        uint64_t size;
    };

    std::string pakPath;
    std::vector<Entry> entries;
};
