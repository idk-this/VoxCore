//
// Created by IDKTHIS on 05.10.2025.
//

#pragma once



#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <deque>
#include <mutex>
#include <optional>
#include <atomic>
#include "Core/Export.h"

using CommandArgs = std::vector<std::string>;
using CommandCallback = std::function<void(const CommandArgs& args)>;


struct VOXCORE_API ConCommand {
    std::string name;
    std::string description;
    CommandCallback callback;
    int flags = 0;
};

class VOXCORE_API ConsoleSystem {
public:
    static ConsoleSystem& Instance();

    void RegisterCommand(const ConCommand& cmd);

    ConCommand* FindCommand(const std::string& name);

    void Execute(const std::string& line);

    void SetAlias(const std::string& name, const std::string& commandLine);
    bool HasAlias(const std::string& name) const;

    void AddHistory(const std::string& line);
    const std::deque<std::string>& GetHistory() const;

    size_t AddListener(const std::string& cmdName, CommandCallback cb);
    void RemoveListener(const std::string& cmdName, size_t listenerId);

    std::vector<std::string> AutoComplete(const std::string& prefix, size_t maxResults = 32) const;

private:
    ConsoleSystem();
    ~ConsoleSystem() = default;

    void ExecuteSingle(const std::string& singleLine);

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, ConCommand> m_commands;
    std::unordered_map<std::string, std::string> m_aliases;
    std::deque<std::string> m_history;
    std::unordered_map<std::string, std::unordered_map<size_t, CommandCallback>> m_listeners;
    std::atomic_size_t m_nextListenerId{1};
};

#define REGISTER_COMMAND_CALLBACK(name, description, lambda) \
static bool CONCAT(_cmd_reg_cb_, __COUNTER__) = [](){ \
ConsoleSystem::Instance().RegisterCommand({name, description, lambda}); \
return true; \
}();

#define REGISTER_COMMAND(name, description) \
static bool CONCAT(_cmd_reg_, __COUNTER__) = [](){ \
ConsoleSystem::Instance().RegisterCommand({name, description, {}}); \
return true; \
}();

#define SUBSCRIBE_COMMAND(cmdName, method) \
size_t CONCAT(_cmd_sub_id_, __COUNTER__) = ConsoleSystem::Instance().AddListener(cmdName, [this](const CommandArgs& args){ this->method(args); });


