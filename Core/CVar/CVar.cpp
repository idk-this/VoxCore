//
// Created by IDKTHIS on 01.07.2025.
//

#include "CVar.h"

#include "Core/Log/Logger.h"
#include "Core/Utils/FileSystem.h"

CONVAR("sv_cheats", 0, "Controls whether cheat-only commands and features are accessible.", CVAR_RUNTIME_ONLY);

ConVar::ConVar(const std::string& name, CVarValue defaultValue, const std::string& description, int flags,
               std::optional<CVarValue> min, std::optional<CVarValue> max)
    : name(name), description(description), value(std::move(defaultValue)), flags(flags), minValue(std::move(min)), maxValue(std::move(max))
{
    if (minValue.has_value() && minValue->index() != value.index()) {
        LOG_ERROR("CVar", "Min value type mismatch for {}", name);
        minValue = std::nullopt;
    }
    if (maxValue.has_value() && maxValue->index() != value.index()) {
        LOG_ERROR("CVar", "Max value type mismatch for {}", name);
        maxValue = std::nullopt;
    }
    Register(this);
}

bool ConVar::SetValue(const CVarValue& newValue)
{
    if (!CheckConstraints(newValue)) return false;
    value = newValue;
    return true;
}

bool ConVar::CheckConstraints(CVarValue newValue) const
{
    if (minValue.has_value() || maxValue.has_value()) {
        if (newValue.index() != value.index()) return false;

        if (std::holds_alternative<int>(newValue)) {
            int newInt = std::get<int>(newValue);
            if (minValue.has_value()) newInt = std::max(newInt, std::get<int>(*minValue));
            if (maxValue.has_value()) newInt = std::min(newInt, std::get<int>(*maxValue));
            return newInt == std::get<int>(newValue);
        }
        else if (std::holds_alternative<float>(newValue)) {
            float newFloat = std::get<float>(newValue);
            if (minValue.has_value()) newFloat = std::max(newFloat, std::get<float>(*minValue));
            if (maxValue.has_value()) newFloat = std::min(newFloat, std::get<float>(*maxValue));
            return newFloat == std::get<float>(newValue);
        }
    }
    return true;
}

void ConVar::Register(ConVar* var) {
    CVarManager::Instance().Register(var);
}

CVarManager& CVarManager::Instance() {
    static CVarManager inst;
    return inst;
}

void CVarManager::Register(ConVar* var) {
    std::lock_guard<std::mutex> lock(mutex_);
    vars[var->name] = var;
}

ConVar* CVarManager::Get(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    return (it != vars.end()) ? it->second : nullptr;
}

void CVarManager::Set(const std::string& name, CVarValue val) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto* var = Get(name);
    if (!var) return;

    if (var->flags & CVAR_READONLY) {
        LOG_ERROR("CVar", "Cannot change READONLY variable: {}", name);
        return;
    }
    if ((var->flags & CVAR_CHEAT)) {
        LOG_ERROR("CVar", "{} can only be changed when cheats are enabled", name);
        return;
    }

    if (!var->SetValue(val)) {
        LOG_ERROR("CVar", "Value out of range for {}", name);
    }
    if ((var->flags & CVAR_ARCHIVE)) {
        //TODO SAVE PATH
    }
}

bool CVarManager::IsReadOnly(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto* var = Get(name);
    return var && (var->flags & CVAR_READONLY);
}

void CVarManager::SaveToFile(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!FileSystem::CreateFileIfNotExists(filename))
    {
        LOG_ERROR("CVar", "Failed to open file for writing: {}", filename);
        return;
    }
    std::ofstream file(filename);

    for (const auto& [name, var] : vars) {
        if (var->flags & CVAR_ARCHIVE) {
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    file << name << " \"" << arg << "\";";
                } else {
                    file << name << " " << arg << ";";
                }
                file << std::endl;
            }, var->value);
        }
    }
}

void CVarManager::LoadFromFile(const std::string& filename)
{
    std::unique_lock lock(mutex_);
    std::ifstream file(filename);
    if (!file.is_open()) {
        lock.unlock();
        SaveToFile(filename);
        LOG_WARN("CVar", "Config file not found: {}", filename);
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    size_t pos = 0;
    while (pos < content.size()) {
        // Skip whitespace
        while (pos < content.size() && std::isspace(content[pos])) pos++;
        if (pos >= content.size()) break;

        size_t name_start = pos;
        while (pos < content.size() && !std::isspace(content[pos]) && content[pos] != ';') pos++;
        std::string name = content.substr(name_start, pos - name_start);

        while (pos < content.size() && std::isspace(content[pos])) pos++;

        std::string valueStr;
        if (pos < content.size() && content[pos] == '"') {
            pos++;
            size_t value_start = pos;
            while (pos < content.size() && content[pos] != '"') pos++;
            valueStr = content.substr(value_start, pos - value_start);
            pos++;
        } else {
            size_t value_start = pos;
            while (pos < content.size() && content[pos] != ';') pos++;
            valueStr = content.substr(value_start, pos - value_start);
        }
        while (pos < content.size() && content[pos] != ';') pos++;
        if (pos < content.size()) pos++;
        lock.unlock();
        if (auto* var = Get(name)) {
            lock.lock();
            try {
                std::visit([&](auto&& current) {
                    using T = std::decay_t<decltype(current)>;
                    if constexpr (std::is_same_v<T, int>) {
                        var->SetValue(std::stoi(valueStr));
                    } else if constexpr (std::is_same_v<T, float>) {
                        var->SetValue(std::stof(valueStr));
                    } else if constexpr (std::is_same_v<T, bool>) {
                        var->SetValue(valueStr == "1" || valueStr == "true");
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        var->SetValue(valueStr);
                    }
                }, var->value);
            } catch (const std::exception& e) {
                LOG_ERROR("CVar", "Failed to parse value for {}: {}", name, e.what());
            }
        }
    }
}
