#include "CVar.h"
#include "Core/Log/Logger.h"
#include <algorithm>

DECLARE_CONVAR("sv_cheats", false, "Controls whether cheat-only commands and features are accessible.", CVAR_RUNTIME_ONLY);

ConVar::ConVar(const std::string& name, CVarValue defaultValue, const std::string& description, int flags,
               std::optional<CVarValue> min, std::optional<CVarValue> max)
    : name(name), defaultValue(defaultValue), description(description), flags(flags), minValue(std::move(min)), maxValue(std::move(max))
{
    CVarRegistry::Instance().RegisterDeclaration(*this);

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
        vars[instance.name] = std::move(instance);
    }
}

ConVarInstance* CVarManager::Find(const std::string& name) {
    auto it = vars.find(name);
    return it != vars.end() ? &it->second : nullptr;
}

CVarValue CVarManager::Get(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    if (it != vars.end()) return it->second.value;
    return {};
}

std::string CVarManager::GetDescription(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    if (it != vars.end()) return it->second.description;
    return "";
}

void CVarManager::Set(const std::string& name, const CVarValue& val) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vars.find(name);
    if (it == vars.end()) return;

    ConVarInstance& var = it->second;

    if (var.flags & CVAR_READONLY) {
        LOG_ERROR("CVar", "Cannot change READONLY variable: {}", name);
        return;
    }

    if ((var.flags & CVAR_CHEAT) && !GET_CVAR(bool, "sv_cheats")) {
        LOG_ERROR("CVar", "{} can only be changed when cheats are enabled", name);
        return;
    }

    if (var.minValue.has_value() || var.maxValue.has_value()) {
        if (val.index() != var.value.index()) {
            LOG_ERROR("CVar", "Type mismatch when setting {}", name);
            return;
        }

        if (std::holds_alternative<int>(val)) {
            int v = std::get<int>(val);
            if (var.minValue.has_value()) v = std::max(v, std::get<int>(*var.minValue));
            if (var.maxValue.has_value()) v = std::min(v, std::get<int>(*var.maxValue));
            if (v != std::get<int>(val)) {
                LOG_ERROR("CVar", "Value out of range for {}", name);
                return;
            }
        } else if (std::holds_alternative<float>(val)) {
            float v = std::get<float>(val);
            if (var.minValue.has_value()) v = std::max(v, std::get<float>(*var.minValue));
            if (var.maxValue.has_value()) v = std::min(v, std::get<float>(*var.maxValue));
            if (v != std::get<float>(val)) {
                LOG_ERROR("CVar", "Value out of range for {}", name);
                return;
            }
        }
    }

    CVarValue oldValue = var.value;
    var.value = val;

    for (auto& cb : var.callbacks) cb(oldValue, val);
    if (var.flags & CVAR_ARCHIVE) {
    }
}
