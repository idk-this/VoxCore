//
// Created by IDKTHIS on 05.10.2025.
//

#include "Console.h"
#include <sstream>
#include <algorithm>
#include "Core/Log/Logger.h"
#include "CVar.h"

ConsoleSystem& ConsoleSystem::Instance() {
    static ConsoleSystem inst;
    return inst;
}

ConsoleSystem::ConsoleSystem() {
    RegisterCommand({"help", "Show available commands", [](const CommandArgs&){
        for (auto& [k, v] : ConsoleSystem::Instance().m_commands) {
            LOG_INFO("Console", "{} - {}", k, v.description);
        }
    }});

    RegisterCommand({"alias", "alias <name> <command...> - create alias", [](const CommandArgs& args){
        if (args.size() < 2) {
            LOG_INFO("Console", "Usage: alias <name> <command...>");
            return;
        }
        std::string name = args[0];
        std::string cmd = args[1];
        for (size_t i = 2; i < args.size(); ++i) cmd += " " + args[i];
        ConsoleSystem::Instance().SetAlias(name, cmd);
        LOG_INFO("Console", "Alias '{}' -> '{}'", name, cmd);
    }});

    RegisterCommand({"history", "Show console history", [](const CommandArgs&){
        const auto& h = ConsoleSystem::Instance().GetHistory();
        size_t i = 0;
        for (auto& line : h) {
            LOG_INFO("Console", "{}: {}", i++, line);
        }
    }});
}

void ConsoleSystem::RegisterCommand(const ConCommand& cmd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_commands[cmd.name] = cmd;
}

ConCommand* ConsoleSystem::FindCommand(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_commands.find(name);
    return it != m_commands.end() ? &it->second : nullptr;
}

void ConsoleSystem::SetAlias(const std::string& name, const std::string& commandLine) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_aliases[name] = commandLine;
}

bool ConsoleSystem::HasAlias(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_aliases.find(name) != m_aliases.end();
}

void ConsoleSystem::AddHistory(const std::string& line) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!line.empty()) {
        m_history.push_front(line);
        if (m_history.size() > 200) m_history.pop_back();
    }
}

const std::deque<std::string>& ConsoleSystem::GetHistory() const {
    return m_history;
}

size_t ConsoleSystem::AddListener(const std::string& cmdName, CommandCallback cb) {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t id = m_nextListenerId++;
    m_listeners[cmdName][id] = cb;
    return id;
}

void ConsoleSystem::RemoveListener(const std::string& cmdName, size_t listenerId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_listeners.find(cmdName);
    if (it != m_listeners.end()) {
        it->second.erase(listenerId);
        if (it->second.empty()) m_listeners.erase(it);
    }
}

std::vector<std::string> ConsoleSystem::AutoComplete(const std::string& prefix, size_t maxResults) const {
    std::vector<std::string> out;
    std::string p = prefix;
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& kv : m_commands) {
        if (kv.first.rfind(p, 0) == 0) {
            out.push_back(kv.first);
            if (out.size() >= maxResults) break;
        }
    }
    for (const auto& kv : m_aliases) {
        if (kv.first.rfind(p, 0) == 0) {
            out.push_back(kv.first);
            if (out.size() >= maxResults) break;
        }
    }
    auto decls = CVarRegistry::Instance().GetDeclarations();
    for (const auto& d : decls) {
        if (d.name.rfind(p, 0) == 0) {
            out.push_back(d.name);
            if (out.size() >= maxResults) break;
        }
    }
    return out;
}

static inline std::vector<std::string> Tokenize(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

void ConsoleSystem::ExecuteSingle(const std::string& singleLine) {
    if (singleLine.empty()) return;

    AddHistory(singleLine);

    auto tokens = Tokenize(singleLine);
    if (tokens.empty()) return;

    std::string name = tokens[0];
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto ait = m_aliases.find(name);
        if (ait != m_aliases.end()) {
            Execute(ait->second);
            return;
        }
    }

    ConCommand* cmd = FindCommand(name);
    if (cmd) {
        if (cmd->callback) {
            try {
                cmd->callback(args);
            } catch (const std::exception& e) {
                LOG_ERROR("Console", "Exception in command '{}': {}", name, e.what());
            }
        }

        std::unordered_map<size_t, CommandCallback> listenersCopy;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_listeners.find(name);
            if (it != m_listeners.end()) listenersCopy = it->second;
        }
        for (auto& kv : listenersCopy) {
            try { kv.second(args); } catch (...) {}
        }
        return;
    }

    if (auto* var = Engine::Application::Get()->GetCVar().Find(name)) {
        if (args.empty()) {
            // get
            auto val = Engine::Application::Get()->GetCVar().Get(name);
            if (std::holds_alternative<int>(val)) LOG_INFO("Console", "{} = {}", name, std::get<int>(val));
            else if (std::holds_alternative<float>(val)) LOG_INFO("Console", "{} = {}", name, std::get<float>(val));
            else if (std::holds_alternative<bool>(val)) LOG_INFO("Console", "{} = {}", name, std::get<bool>(val));
            else if (std::holds_alternative<std::string>(val)) LOG_INFO("Console", "{} = {}", name, std::get<std::string>(val));
            return;
        } else {
            CVarValue newVal;
            if (std::holds_alternative<int>(var->value)) {
                try { newVal = std::stoi(args[0]); }
                catch (...) { LOG_ERROR("Console", "Invalid int for {}", name); return; }
            } else if (std::holds_alternative<float>(var->value)) {
                try { newVal = std::stof(args[0]); }
                catch (...) { LOG_ERROR("Console", "Invalid float for {}", name); return; }
            } else if (std::holds_alternative<bool>(var->value)) {
                std::string a = args[0];
                std::transform(a.begin(), a.end(), a.begin(), ::tolower);
                if (a == "1" || a == "true" || a == "yes") newVal = true;
                else if (a == "0" || a == "false" || a == "no") newVal = false;
                else { LOG_ERROR("Console", "Invalid bool for {}", name); return; }
            } else if (std::holds_alternative<std::string>(var->value)) {
                std::string s = args.empty() ? "" : args[0];
                for (size_t i = 1; i < args.size(); ++i) s += " " + args[i];
                newVal = s;
            } else {
                LOG_ERROR("Console", "Unknown cvar type for {}", name);
                return;
            }
            Engine::Application::Get()->GetCVar().Set(name, newVal, CVarSetSource::Console);
            return;
        }
    }

    LOG_ERROR("Console", "Unknown command or cvar: {}", name);
}

void ConsoleSystem::Execute(const std::string& line) {
    std::istringstream ss(line);
    std::string segment;
    while (std::getline(ss, segment, ';')) {
        size_t start = segment.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = segment.find_last_not_of(" \t\r\n");
        std::string trimmed = segment.substr(start, end - start + 1);
        ExecuteSingle(trimmed);
    }
}