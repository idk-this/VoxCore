//
// Created by IDKTHIS on 01.07.2025.
//

#ifndef CVAR_H
#define CVAR_H

#include <string>
#include <unordered_map>
#include <variant>
#include <functional>
#include <iostream>
#include <mutex>
#include <memory>
#include "Flags.h"
#include "Core/Export.h"
using CVarValue = std::variant<int, float, bool, std::string>;

struct ConVar {
    std::string name;
    std::string description;
    CVarValue value;
    int flags;

    ConVar(const std::string& name, CVarValue defaultValue, const std::string& description, int flags);

    template<typename T>
    T Get() const { return std::get<T>(value); }

    template<typename T>
    void Set(T newValue) { value = newValue; }

    static void Register(ConVar* var);
};

class VOXCORE_API CVarManager {
public:
    static CVarManager& Instance();

    void Register(ConVar* var);
    ConVar* Get(const std::string& name);

    void Set(const std::string& name, CVarValue val);

    bool IsReadOnly(const std::string& name);

private:
    std::unordered_map<std::string, ConVar*> vars;
    std::mutex mutex_;
};

#define CONCAT2(x, y) x##y
#define CONCAT(x, y) CONCAT2(x, y)
#define CONVAR(name, defaultVal, desc, flags) \
static ConVar CONCAT(cvar_, __LINE__)(name, defaultVal, desc, flags)
#define GET_CVAR(type, name) \
(CVarManager::Instance().Get(name) ? std::get<type>(CVarManager::Instance().Get(name)->value) : type{})
#define GET_CVAR_DESC(name) \
(CVarManager::Instance().Get(name) ? CVarManager::Instance().Get(name)->description : "")
#define SET_CVAR(name, newVal) \
    do { \
        auto* var = CVarManager::Instance().Get(name); \
        if (var) var->Set(newVal); \
    } while (0)


#endif //CVAR_H
