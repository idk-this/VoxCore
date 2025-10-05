#ifndef CVAR_H
#define CVAR_H

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>
#include <mutex>
#include "Flags.h"
#include "Application/Application.h"
#include "Core/Export.h"

using CVarValue = std::variant<int, float, bool, std::string>;
using CVarCallback = std::function<void(const CVarValue& oldValue, const CVarValue& newValue)>;
enum class CVarSetSource {
    Program = 0,
    Console = 1
};

class VOXCORE_API ConVar {
public:
    std::string name;
    std::string description;
    CVarValue defaultValue;
    int flags;
    std::optional<CVarValue> minValue;
    std::optional<CVarValue> maxValue;

    ConVar(const std::string& name, CVarValue defaultValue, const std::string& description, int flags,
           std::optional<CVarValue> min = std::nullopt, std::optional<CVarValue> max = std::nullopt);
};

class VOXCORE_API CVarRegistry {
public:
    static CVarRegistry& Instance() {
        static CVarRegistry inst;
        return inst;
    }

    void RegisterDeclaration(const ConVar& def) {
        declarations.push_back(def);
    }

    const std::vector<ConVar>& GetDeclarations() const {
        return declarations;
    }

private:
    std::vector<ConVar> declarations;
};

struct VOXCORE_API ConVarInstance {
    std::string name;
    std::string description;
    CVarValue value;
    int flags;
    std::optional<CVarValue> minValue;
    std::optional<CVarValue> maxValue;
    std::vector<CVarCallback> callbacks;
};

class VOXCORE_API CVarManager {
public:
    CVarManager(const std::vector<ConVar>& declaredVars);
    void Set(const std::string& name, const CVarValue& val, CVarSetSource source = CVarSetSource::Program);

    CVarValue Get(const std::string& name) const;
    std::string GetDescription(const std::string& name) const;
    ConVarInstance* Find(const std::string& name);

private:
    std::unordered_map<std::string, ConVarInstance> vars;
    mutable std::mutex mutex_;
};

#define CONCAT2(x, y) x##y
#define CONCAT(x, y) CONCAT2(x, y)

#define DECLARE_CONVAR(name, defaultVal, desc, flags) \
static ConVar CONCAT(cvar_, __COUNTER__)(name, defaultVal, desc, flags)

#define DECLARE_CONVAR_MINMAX(name, defaultVal, minVal, maxVal, desc, flags) \
static ConVar CONCAT(cvar_, __COUNTER__)( \
std::string(name), \
CVarValue(defaultVal), \
std::string(desc), \
flags, \
CVarValue(minVal), \
CVarValue(maxVal))

#define DECLARE_CVAR_CALLBACK(cvarName, func) \
do { \
    if (auto* var = Engine::Application::Get()->GetCVar().Find(cvarName)) { \
        var->callbacks.push_back(func); \
    } \
} while(0)

#define GET_CVAR(type, name) \
([]() -> type { \
auto val = Engine::Application::Get()->GetCVar().Get(name); \
if (std::holds_alternative<type>(val)) return std::get<type>(val); \
return type{}; \
}())

#define GET_CVAR_DESC(name) \
(Engine::Application::Get()->GetCVar().GetDescription(name))

#define SET_CVAR(name, newVal) \
do { Engine::Application::Get()->GetCVar().Set(name, newVal); } while(0)

#endif // CVAR_H
