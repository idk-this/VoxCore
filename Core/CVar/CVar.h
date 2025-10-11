#ifndef CVAR_H
#define CVAR_H

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>
#include <mutex>
#include <memory>
#include <sstream>
#include "Flags.h"
#include "Application/Application.h"
#include "Core/Export.h"
#include "Core/Log/Logger.h"

using CVarValue = std::variant<int, float, bool, std::string>;
using CVarCallback = std::function<void(const CVarValue& oldValue, const CVarValue& newValue)>;

enum class CVarSetSource {
    Program = 0,
    Console = 1,
    Config = 2
};

enum class CVarType {
    Integer,
    Float,
    Boolean,
    String
};
struct CallbackEntry {
    size_t id;
    CVarCallback func;
};
class VOXCORE_API ConVar {
public:
    std::string name;
    std::string description;
    CVarValue defaultValue;
    int flags;
    std::optional<CVarValue> minValue;
    std::optional<CVarValue> maxValue;
    CVarType type;

    ConVar(const std::string& name, CVarValue defaultValue, const std::string& description, int flags,
           std::optional<CVarValue> min = std::nullopt, std::optional<CVarValue> max = std::nullopt);

    std::string ToString() const;
    CVarType GetType() const { return type; }
};

class VOXCORE_API CVarRegistry {
public:
    static CVarRegistry& Instance() {
        static CVarRegistry inst;
        return inst;
    }

    void RegisterDeclaration(const ConVar& def) {
        std::lock_guard<std::mutex> lock(mutex_);
        declarations.push_back(def);
        declarationsMap[def.name] = &declarations.back();
    }

    const std::vector<ConVar>& GetDeclarations() const {
        return declarations;
    }

    const ConVar* FindDeclaration(const std::string& name) const {
        auto it = declarationsMap.find(name);
        return it != declarationsMap.end() ? it->second : nullptr;
    }

    std::vector<std::string> GetMatchingNames(const std::string& prefix) const {
        std::vector<std::string> matches;
        for (const auto& decl : declarations) {
            if (decl.name.find(prefix) == 0) {
                matches.push_back(decl.name);
            }
        }
        return matches;
    }

private:
    std::vector<ConVar> declarations;
    std::unordered_map<std::string, ConVar*> declarationsMap;
    mutable std::mutex mutex_;
};

struct VOXCORE_API ConVarInstance {
    std::string name;
    std::string description;
    CVarValue value;
    int flags;
    std::optional<CVarValue> minValue;
    std::optional<CVarValue> maxValue;
    CVarType type;
    std::vector<CallbackEntry> callbacks;

    std::string ValueToString() const;
    bool SetValueFromString(const std::string& str, CVarSetSource source = CVarSetSource::Console);
};

class VOXCORE_API CVarManager {
public:
    CVarManager(const std::vector<ConVar>& declaredVars);
    void Set(const std::string& name, const CVarValue& val, CVarSetSource source = CVarSetSource::Program);
    bool SetFromString(const std::string& name, const std::string& value, CVarSetSource source = CVarSetSource::Console);

    CVarValue Get(const std::string& name) const;
    std::string GetAsString(const std::string& name) const;
    std::string GetDescription(const std::string& name) const;
    ConVarInstance* Find(const std::string& name);
    const ConVarInstance* Find(const std::string& name) const;

    size_t AddCallback(const std::string& name, CVarCallback callback);
    void RemoveCallback(const std::string& name, size_t callbackId);

    std::vector<std::string> GetMatchingNames(const std::string& prefix) const;
    std::unordered_map<std::string, ConVarInstance> GetAllVars() const { return vars; }

    bool SaveToFile(const std::string& filename, bool onlyArchived = true) const;
    bool LoadFromFile(const std::string& filename);

private:
    std::unordered_map<std::string, ConVarInstance> vars;
    mutable std::mutex mutex_;

    bool ValidateValue(const ConVarInstance& var, const CVarValue& val, CVarSetSource source) const;
};

#define CONCAT2(x, y) x##y
#define CONCAT(x, y) CONCAT2(x, y)

#define DECLARE_CONVAR(name, defaultVal, desc, flags) \
static ConVar CONCAT(cvar_, __COUNTER__)(name, defaultVal, desc, flags)

#define DECLARE_CONVAR_MINMAX(name, defaultVal, minVal, maxVal, desc, flags) \
static ConVar CONCAT(cvar_, __COUNTER__)(name, defaultVal, desc, flags, minVal, maxVal)

#define DECLARE_CVAR_CALLBACK(cvarName, func) \
do { \
auto& cvarMgr = Engine::Application::Get()->GetCVar(); \
if (cvarMgr.Find(cvarName)) { \
cvarMgr.AddCallback(cvarName, func); \
} else { \
LOG_WARN("CVar", "Cannot register callback for non-existent CVar: {}", cvarName); \
} \
} while (0)



#define GET_CVAR(type, name) \
([]() -> type { \
auto val = Engine::Application::Get()->GetCVar().Get(name); \
if (std::holds_alternative<type>(val)) return std::get<type>(val); \
LOG_ERROR("CVar", "Type mismatch when getting CVar '{}'", name); \
return type{}; \
}())

#define GET_CVAR_SAFE(type, name, defaultValue) \
([]() -> type { \
auto val = Engine::Application::Get()->GetCVar().Get(name); \
if (std::holds_alternative<type>(val)) return std::get<type>(val); \
return defaultValue; \
}())

#define GET_CVAR_DESC(name) \
(Engine::Application::Get()->GetCVar().GetDescription(name))

#define SET_CVAR(name, newVal) \
do { Engine::Application::Get()->GetCVar().Set(name, newVal); } while(0)

#define SET_CVAR_STRING(name, newValStr) \
do { Engine::Application::Get()->GetCVar().SetFromString(name, newValStr); } while(0)

#endif // CVAR_H