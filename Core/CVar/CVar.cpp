#include "CVar.h"
#include "Core/Log/Logger.h"
#include <algorithm>
#include <fstream>
#include <iomanip>

DECLARE_CONVAR("sv_cheats", false, "Controls whether cheat-only commands and features are accessible.", CVAR_RUNTIME_ONLY);

ConVar::ConVar(const std::string& name, CVarValue defaultValue, const std::string& description, int flags,
               std::optional<CVarValue> min, std::optional<CVarValue> max)
    : name(name), defaultValue(defaultValue), description(description), flags(flags),
      minValue(std::move(min)), maxValue(std::move(max))
{
    // Determine type from defaultValue
    if (std::holds_alternative<int>(defaultValue)) type = CVarType::Integer;
    else if (std::holds_alternative<float>(defaultValue)) type = CVarType::Float;
    else if (std::holds_alternative<bool>(defaultValue)) type = CVarType::Boolean;
    else if (std::holds_alternative<std::string>(defaultValue)) type = CVarType::String;

    CVarRegistry::Instance().RegisterDeclaration(*this);
}

std::string ConVar::ToString() const {
    std::stringstream ss;
    ss << name << " = ";
    if (std::holds_alternative<int>(defaultValue)) ss << std::get<int>(defaultValue);
    else if (std::holds_alternative<float>(defaultValue)) ss << std::get<float>(defaultValue);
    else if (std::holds_alternative<bool>(defaultValue)) ss << (std::get<bool>(defaultValue) ? "true" : "false");
    else if (std::holds_alternative<std::string>(defaultValue)) ss << "\"" << std::get<std::string>(defaultValue) << "\"";

    if (minValue.has_value() || maxValue.has_value()) {
        ss << " [";
        if (minValue.has_value()) {
            if (std::holds_alternative<int>(*minValue)) ss << std::get<int>(*minValue);
            else if (std::holds_alternative<float>(*minValue)) ss << std::get<float>(*minValue);
        }
        ss << "..";
        if (maxValue.has_value()) {
            if (std::holds_alternative<int>(*maxValue)) ss << std::get<int>(*maxValue);
            else if (std::holds_alternative<float>(*maxValue)) ss << std::get<float>(*maxValue);
        }
        ss << "]";
    }

    ss << " // " << description;
    return ss.str();
}

CVarManager::CVarManager(const std::vector<ConVar>& declaredVars) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& decl : declaredVars) {
        ConVarInstance instance;
        instance.name = decl.name;
        instance.description = decl.description;
        instance.value = decl.defaultValue;
        instance.flags = decl.flags;
        instance.minValue = decl.minValue;
        instance.maxValue = decl.maxValue;
        instance.type = decl.type;
        vars[instance.name] = std::move(instance);
    }
}

ConVarInstance* CVarManager::Find(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    return it != vars.end() ? &it->second : nullptr;
}

const ConVarInstance* CVarManager::Find(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    return it != vars.end() ? &it->second : nullptr;
}

CVarValue CVarManager::Get(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    if (it != vars.end()) return it->second.value;
    LOG_WARN("CVar", "Attempt to get non-existent CVar: {}", name);
    return {};
}

std::string CVarManager::GetAsString(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    if (it != vars.end()) {
        return it->second.ValueToString();
    }
    return "";
}

std::string CVarManager::GetDescription(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    if (it != vars.end()) return it->second.description;
    return "";
}

bool CVarManager::ValidateValue(const ConVarInstance& var, const CVarValue& val, CVarSetSource source) const {
    // Check console edit permission
    if (source == CVarSetSource::Console) {
        if (!(var.flags & CVAR_CONSOLE_EDIT)) {
            LOG_ERROR("CVar", "Cannot change variable '{}' from console (no CVAR_CONSOLE_EDIT)", var.name);
            return false;
        }
    }

    // Check readonly
    if (var.flags & CVAR_READONLY) {
        LOG_ERROR("CVar", "Cannot change READONLY variable: {}", var.name);
        return false;
    }

    // Check cheat protection
    if ((var.flags & CVAR_CHEAT) && !GET_CVAR(bool, "sv_cheats")) {
        LOG_ERROR("CVar", "{} can only be changed when cheats are enabled", var.name);
        return false;
    }

    // Type check
    if (val.index() != var.value.index()) {
        LOG_ERROR("CVar", "Type mismatch when setting {}", var.name);
        return false;
    }

    // Range validation
    if (var.minValue.has_value() || var.maxValue.has_value()) {
        if (std::holds_alternative<int>(val)) {
            int v = std::get<int>(val);
            if (var.minValue.has_value()) v = std::max(v, std::get<int>(*var.minValue));
            if (var.maxValue.has_value()) v = std::min(v, std::get<int>(*var.maxValue));
            if (v != std::get<int>(val)) {
                LOG_ERROR("CVar", "Value out of range for {}: {} not in [{}, {}]",
                         var.name, std::get<int>(val),
                         var.minValue.has_value() ? std::get<int>(*var.minValue) : INT_MIN,
                         var.maxValue.has_value() ? std::get<int>(*var.maxValue) : INT_MAX);
                return false;
            }
        } else if (std::holds_alternative<float>(val)) {
            float v = std::get<float>(val);
            if (var.minValue.has_value()) v = std::max(v, std::get<float>(*var.minValue));
            if (var.maxValue.has_value()) v = std::min(v, std::get<float>(*var.maxValue));
            if (v != std::get<float>(val)) {
                LOG_ERROR("CVar", "Value out of range for {}: {} not in [{}, {}]",
                         var.name, std::get<float>(val),
                         var.minValue.has_value() ? std::get<float>(*var.minValue) : -FLT_MAX,
                         var.maxValue.has_value() ? std::get<float>(*var.maxValue) : FLT_MAX);
                return false;
            }
        }
    }

    return true;
}

void CVarManager::Set(const std::string& name, const CVarValue& val, CVarSetSource source) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    if (it == vars.end()) {
        LOG_ERROR("CVar", "Attempt to set non-existent CVar: {}", name);
        return;
    }

    ConVarInstance& var = it->second;

    if (!ValidateValue(var, val, source)) {
        return;
    }

    CVarValue oldValue = var.value;
    var.value = val;

    // Execute callbacks
    for (auto& entry : var.callbacks) {
        try {
            entry.func(oldValue, val);
        } catch (const std::exception& e) {
            LOG_ERROR("CVar", "Exception in callback for {} (id {}): {}", name, entry.id, e.what());
        }
    }

    // Archive if needed
    if (var.flags & CVAR_ARCHIVE) {
        // Future: auto-save to config
    }
}

bool CVarManager::SetFromString(const std::string& name, const std::string& value, CVarSetSource source) {
    auto* var = Find(name);
    if (!var) {
        LOG_ERROR("CVar", "CVar not found: {}", name);
        return false;
    }

    return var->SetValueFromString(value, source);
}

size_t CVarManager::AddCallback(const std::string& name, CVarCallback callback) {
    static size_t nextCallbackId = 1;
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    if (it != vars.end()) {
        size_t id = nextCallbackId++;
        it->second.callbacks.push_back({ id, std::move(callback) });
        return id;
    }
    return 0;
}


void CVarManager::RemoveCallback(const std::string& name, size_t callbackId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    if (it != vars.end()) {
        auto& callbacks = it->second.callbacks;
        callbacks.erase(
            std::remove_if(callbacks.begin(), callbacks.end(),
                [callbackId](const CallbackEntry& e) { return e.id == callbackId; }),
            callbacks.end()
        );
    }
}

std::vector<std::string> CVarManager::GetMatchingNames(const std::string& prefix) const {
    std::vector<std::string> matches;
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& pair : vars) {
        if (pair.first.find(prefix) == 0) {
            matches.push_back(pair.first);
        }
    }
    return matches;
}

bool CVarManager::SaveToFile(const std::string& filename, bool onlyArchived) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        LOG_ERROR("CVar", "Failed to open file for writing: {}", filename);
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    file << "// CVar configuration file\n";
    file << "// Generated automatically\n\n";

    for (const auto& pair : vars) {
        if (!onlyArchived || (pair.second.flags & CVAR_ARCHIVE)) {
            file << pair.first << " \"" << pair.second.ValueToString() << "\"\n";
        }
    }

    file.close();
    LOG_INFO("CVar", "Saved CVars to: {}", filename);
    return true;
}

bool CVarManager::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_WARN("CVar", "Could not open CVar config file: {}", filename);
        return false;
    }

    std::string line;
    int lineNum = 0;
    int loadedCount = 0;

    while (std::getline(file, line)) {
        lineNum++;

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == '/') continue;

        std::istringstream iss(line);
        std::string name, value;

        if (iss >> name) {
            // Read the rest as value, handling quotes
            std::getline(iss, value);

            // Trim whitespace and quotes
            size_t start = value.find_first_not_of(" \t\"");
            size_t end = value.find_last_not_of(" \t\"");

            if (start != std::string::npos && end != std::string::npos) {
                value = value.substr(start, end - start + 1);

                if (SetFromString(name, value, CVarSetSource::Config)) {
                    loadedCount++;
                } else {
                    LOG_WARN("CVar", "Failed to set CVar '{}' from config (line {})", name, lineNum);
                }
            }
        }
    }

    file.close();
    LOG_INFO("CVar", "Loaded {} CVars from: {}", loadedCount, filename);
    return loadedCount > 0;
}

std::string ConVarInstance::ValueToString() const {
    if (std::holds_alternative<int>(value)) return std::to_string(std::get<int>(value));
    else if (std::holds_alternative<float>(value)) return std::to_string(std::get<float>(value));
    else if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? "true" : "false";
    else if (std::holds_alternative<std::string>(value)) return std::get<std::string>(value);
    return "";
}

bool ConVarInstance::SetValueFromString(const std::string& str, CVarSetSource source) {
    CVarValue newValue;

    try {
        if (std::holds_alternative<int>(value)) {
            newValue = std::stoi(str);
        } else if (std::holds_alternative<float>(value)) {
            newValue = std::stof(str);
        } else if (std::holds_alternative<bool>(value)) {
            std::string lowerStr = str;
            std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
            newValue = (lowerStr == "true" || lowerStr == "1" || lowerStr == "yes" || lowerStr == "on");
        } else if (std::holds_alternative<std::string>(value)) {
            newValue = str;
        } else {
            return false;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("CVar", "Failed to parse value '{}' for {}: {}", str, name, e.what());
        return false;
    }

    // Use CVarManager to set with validation
    auto* cvarManager = &Engine::Application::Get()->GetCVar();
    cvarManager->Set(name, newValue, source);
    return true;
}