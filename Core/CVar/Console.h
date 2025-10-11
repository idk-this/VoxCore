#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <deque>
#include <mutex>
#include <optional>
#include <atomic>
#include <regex>
#include "Core/Export.h"

using CommandArgs = std::vector<std::string>;
using CommandCallback = std::function<void(const CommandArgs& args)>;

struct VOXCORE_API ConCommand {
    std::string name;
    std::string description;
    CommandCallback callback;
    std::string usage;
    int flags = 0;

    ConCommand(const std::string& n, const std::string& desc, CommandCallback cb,
               const std::string& use = "", int f = 0)
        : name(n), description(desc), callback(cb), usage(use), flags(f) {}
};

class VOXCORE_API ConsoleSystem {
public:
    static ConsoleSystem& Instance();

    void RegisterCommand(const ConCommand& cmd);
    void UnregisterCommand(const std::string& name);

    ConCommand* FindCommand(const std::string& name);
    const std::unordered_map<std::string, ConCommand>& GetCommands() const { return m_commands; }

    void Execute(const std::string& line);
    void ExecuteFile(const std::string& filename);

    // Enhanced execution with output capture
    std::string ExecuteWithResult(const std::string& line);

    void SetAlias(const std::string& name, const std::string& commandLine);
    void RemoveAlias(const std::string& name);
    bool HasAlias(const std::string& name);
    std::string GetAlias(const std::string& name);

    void AddHistory(const std::string& line);
    const std::deque<std::string>& GetHistory();
    void ClearHistory();

    // Enhanced history search
    std::string SearchHistory(const std::string& prefix, bool reverse = false);

    size_t AddListener(const std::string& cmdName, CommandCallback cb);
    void RemoveListener(const std::string& cmdName, size_t listenerId);

    std::vector<std::string> AutoComplete(const std::string& prefix, size_t maxResults = 32);

    // Enhanced suggestions with type information
    struct Suggestion {
        std::string name;
        std::string type; // "command", "cvar", "alias"
        std::string description;
    };
    std::vector<Suggestion> GetSuggestions(const std::string& prefix);

    void SetOutputCallback(std::function<void(const std::string&)> callback) {
        m_outputCallback = callback;
    }

    void Print(const std::string& message);

private:
    ConsoleSystem();
    ~ConsoleSystem() = default;

    void ExecuteSingle(const std::string& singleLine);
    void ExecuteSingle(const std::string& singleLine, int depth);

    std::recursive_mutex m_mutex;
    std::unordered_map<std::string, ConCommand> m_commands;
    std::unordered_map<std::string, std::string> m_aliases;
    std::deque<std::string> m_history;
    std::unordered_map<std::string, std::unordered_map<size_t, CommandCallback>> m_listeners;
    std::atomic_size_t m_nextListenerId{1};
    std::function<void(const std::string&)> m_outputCallback;

    // History navigation state
    mutable int m_historyNavigationIndex{-1};
    mutable std::string m_currentInputBuffer;
};

// Improved parsing utilities
inline bool Parse3Floats(const CommandArgs& args, float& x, float& y, float& z) {
    if (args.size() != 3) return false;

    try {
        x = std::stof(args[0]);
        y = std::stof(args[1]);
        z = std::stof(args[2]);
    } catch (...) {
        return false;
    }
    return true;
}

inline bool ParseInt(const std::string& str, int& value) {
    try {
        value = std::stoi(str);
        return true;
    } catch (...) {
        return false;
    }
}

inline bool ParseFloat(const std::string& str, float& value) {
    try {
        value = std::stof(str);
        return true;
    } catch (...) {
        return false;
    }
}

inline bool ParseBool(const std::string& str, bool& value) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "true" || lower == "1" || lower == "yes" || lower == "on") {
        value = true;
        return true;
    } else if (lower == "false" || lower == "0" || lower == "no" || lower == "off") {
        value = false;
        return true;
    }
    return false;
}

// Enhanced registration macros
#define REGISTER_COMMAND_FULL(name, description, usage, flags, lambda) \
static bool CONCAT(_cmd_reg_cb_, __COUNTER__) = [](){ \
    ConsoleSystem::Instance().RegisterCommand({name, description, lambda, usage, flags}); \
    return true; \
}()

#define REGISTER_COMMAND_CALLBACK(name, description, lambda) \
REGISTER_COMMAND_FULL(name, description, "", 0, lambda)

#define REGISTER_COMMAND(name, description) \
REGISTER_COMMAND_FULL(name, description, "", 0, [](const CommandArgs&){})

#define REGISTER_COMMANDF(name, description, flags) \
REGISTER_COMMAND_FULL(name, description, "", flags, [](const CommandArgs&){}) \

#define REGISTER_COMMAND_USAGE(name, description, usage) \
REGISTER_COMMAND_FULL(name, description, usage, 0, [](const CommandArgs& args)

#define SUBSCRIBE_COMMAND(cmdName, method) \
size_t CONCAT(_cmd_sub_id_, __COUNTER__) = ConsoleSystem::Instance().AddListener(cmdName, [this](const CommandArgs& args){ this->method(args); })

// Quick command registration for common tasks
#define REGISTER_HELP_COMMAND(name, helpText) \
REGISTER_COMMAND_CALLBACK(name, "Show help for " name, [](const CommandArgs&){ \
    ConsoleSystem::Instance().Print(helpText); \
})

// Command flags
enum ConsoleCommandFlags {
    CMD_CHEAT = 1 << 0,        // Requires sv_cheats
    CMD_DEVELOPER = 1 << 1,    // Only in developer mode
    CMD_HIDDEN = 1 << 2,       // Hidden from help and autocomplete
    CMD_SERVER = 1 << 3,       // Server-only command
    CMD_CLIENT = 1 << 4,       // Client-only command
};