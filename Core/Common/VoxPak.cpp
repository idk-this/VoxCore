//
// Created by IDKTHIS on 09.09.2025.
//

#include "VoxPak.h"
#include <fstream>
#include <filesystem>
#include <string.h>

#include "Core/CVar/CVar.h"
#include "Core/Log/Logger.h"
#include "Core/Utils/FileSystem.h"
namespace fs = std::filesystem;

DECLARE_CONVAR("sv_allow_modding", true, "Allow loading files directly from the filesystem instead of using packed assets", CVAR_READONLY);


bool VoxPak::Open(const std::string& pakFile) {
    pakPath = pakFile;
    std::ifstream ifs(pakFile, std::ios::binary);
    if (!ifs) return false;

    FileHeader hdr{};
    ifs.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    if (strncmp(hdr.magic, "VOXPAK", 6) != 0) return false;

    entries.clear();
    for (uint32_t i = 0; i < hdr.fileCount; i++) {
        uint16_t len;
        ifs.read(reinterpret_cast<char*>(&len), sizeof(len));
        std::string path(len, '\0');
        ifs.read(path.data(), len);
        uint64_t off, sz;
        ifs.read(reinterpret_cast<char*>(&off), sizeof(off));
        ifs.read(reinterpret_cast<char*>(&sz), sizeof(sz));
        entries.push_back({path, off, sz});
    }
    pakModOverride.clear();
    bool allowMods = GET_CVAR(bool, "sv_allow_modding");
    if (!allowMods)
    {
        LOG_INFO("VoxPak", "Opened {} with {} files. Mods disabled.", pakFile, entries.size());
        return true;
    }
    int modCount = 0;
    std::filesystem::path modsPath(FileSystem::GetWorkingDirectory() + "Content/Mods");
    std::string pakName = std::filesystem::path(pakFile).stem().string();

    if (!std::filesystem::exists(modsPath) || !std::filesystem::is_directory(modsPath))
    {
        LOG_INFO("VoxPak", "Opened {} with {} files. {} mods override this pak.", pakFile, entries.size(), modCount);
        return true;
    }
    for (auto& modDir : std::filesystem::directory_iterator(modsPath)) {
        if (!modDir.is_directory()) continue;

        std::filesystem::path pakDir = modDir.path() / pakName;
        if (std::filesystem::exists(pakDir) && std::filesystem::is_directory(pakDir)) {
            pakModOverride.push_back(pakDir);
            modCount++;
        }
    }
    LOG_INFO("VoxPak", "Opened {} with {} files. {} mods override this pak.", pakFile, entries.size(), modCount);
    return true;
}

std::vector<std::string> VoxPak::ListFiles() const {
    std::vector<std::string> out;
    for(auto& e : entries) out.push_back(e.path);
    return out;
}

std::vector<uint8_t> VoxPak::ReadFile(const std::string& path) const {
    std::ifstream ifs(pakPath, std::ios::binary);
    if(!ifs) return {};
    for(auto& e : entries) {
        if(e.path == path) {
            std::vector<uint8_t> buf(e.size);
            ifs.seekg(e.offset, std::ios::beg);
            ifs.read(reinterpret_cast<char*>(buf.data()), e.size);
            return buf;
        }
    }
    return {};
}

std::vector<uint8_t> VoxPak::ReadFileWithOverride(const std::string& path) const {
    if (!GET_CVAR(bool, "sv_allow_modding")) return ReadFile(path);

    for (auto& pakDir : pakModOverride) {
        std::filesystem::path fullPath = pakDir / path;
        if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath)) {
            std::ifstream ifs(fullPath, std::ios::binary);
            if (ifs) {
                return std::vector<uint8_t>(std::istreambuf_iterator<char>(ifs), {});
            }
        }
    }

    return ReadFile(path);
}

std::string VoxPak::ReadFileWithOverrideString(const std::string& path) const
{
    std::vector<uint8_t> result = ReadFileWithOverride(path);
    return std::string(result.begin(), result.end());
}
