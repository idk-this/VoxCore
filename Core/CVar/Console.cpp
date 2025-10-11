#include "Console.h"
#include <sstream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <iomanip>
#include "Core/Log/Logger.h"
#include "CVar.h"

ConsoleSystem& ConsoleSystem::Instance() {
    static ConsoleSystem inst;
    return inst;
}

ConsoleSystem::ConsoleSystem() {
    // Register enhanced help command
    RegisterCommand({"help", "Show help for commands and cvars", [](const CommandArgs& args){
        auto& console = ConsoleSystem::Instance();
        
        if (args.empty()) {
            console.Print("Available commands:");
            console.Print("  help [command] - Show help for specific command");
            console.Print("  help cvars - List all CVars");
            console.Print("  help commands - List all commands");
            console.Print("  alias <name> <command> - Create alias");
            console.Print("  history - Show command history");
            console.Print("  clear - Clear console history");
            console.Print("  exec <file> - Execute script file");
            console.Print("  echo <text> - Print text to console");
            return;
        }
        
        std::string what = args[0];
        if (what == "cvars") {
            auto decls = CVarRegistry::Instance().GetDeclarations();
            console.Print("Available CVars (" + std::to_string(decls.size()) + "):");
            for (const auto& decl : decls) {
                console.Print("  " + decl.ToString());
            }
        } else if (what == "commands") {
            auto commands = console.GetCommands();
            console.Print("Available commands (" + std::to_string(commands.size()) + "):");
            for (const auto& [name, cmd] : commands) {
                if (!(cmd.flags & CMD_HIDDEN)) {
                    console.Print("  " + name + " - " + cmd.description);
                    if (!cmd.usage.empty()) {
                        console.Print("    Usage: " + cmd.usage);
                    }
                }
            }
        } else {
            // Look for specific command or cvar
            if (auto* cmd = console.FindCommand(what)) {
                console.Print(cmd->name + " - " + cmd->description);
                if (!cmd->usage.empty()) {
                    console.Print("Usage: " + cmd->usage);
                }
            } else if (auto* cvar = CVarRegistry::Instance().FindDeclaration(what)) {
                console.Print(cvar->ToString());
            } else {
                console.Print("No help found for: " + what);
            }
        }
    }});

    RegisterCommand({"alias", "Create command alias", [](const CommandArgs& args){
        if (args.size() < 2) {
            ConsoleSystem::Instance().Print("Usage: alias <name> <command...>");
            return;
        }
        std::string name = args[0];
        std::string cmd = args[1];
        for (size_t i = 2; i < args.size(); ++i) cmd += " " + args[i];
        ConsoleSystem::Instance().SetAlias(name, cmd);
        ConsoleSystem::Instance().Print("Alias '" + name + "' -> '" + cmd + "'");
    }, "alias <name> <command...>"});

    RegisterCommand({"history", "Show command history", [](const CommandArgs&){
        const auto& h = ConsoleSystem::Instance().GetHistory();
        if (h.empty()) {
            ConsoleSystem::Instance().Print("History is empty");
            return;
        }
        size_t i = 0;
        for (auto& line : h) {
            ConsoleSystem::Instance().Print(std::to_string(i++) + ": " + line);
        }
    }});

    RegisterCommand({"clear", "Clear console history", [](const CommandArgs&){
        ConsoleSystem::Instance().ClearHistory();
        ConsoleSystem::Instance().Print("Console history cleared");
    }});

    RegisterCommand({"exec", "Execute script file", [](const CommandArgs& args){
        if (args.empty()) {
            ConsoleSystem::Instance().Print("Usage: exec <filename>");
            return;
        }
        ConsoleSystem::Instance().ExecuteFile(args[0]);
    }, "exec <filename>"});

    RegisterCommand({"echo", "Print text to console", [](const CommandArgs& args){
        std::string text;
        for (const auto& arg : args) text += arg + " ";
        ConsoleSystem::Instance().Print(text);
    }, "echo <text>"});

    RegisterCommand({"cvarlist", "List all CVars", [](const CommandArgs& args){
        bool showDescriptions = true;
        std::string filter;
        
        if (!args.empty()) {
            if (args[0] == "nodefault" || args[0] == "brief") {
                showDescriptions = false;
            } else {
                filter = args[0];
            }
        }
        
        auto decls = CVarRegistry::Instance().GetDeclarations();
        int count = 0;
        
        for (const auto& decl : decls) {
            if (!filter.empty() && decl.name.find(filter) == std::string::npos) {
                continue;
            }
            
            if (showDescriptions) {
                ConsoleSystem::Instance().Print(decl.ToString());
            } else {
                ConsoleSystem::Instance().Print(decl.name + " = " + decl.ToString());
            }
            count++;
        }
        
        ConsoleSystem::Instance().Print("Total CVars: " + std::to_string(count));
    }, "cvarlist [filter|brief|nodefault]"});
}

void ConsoleSystem::RegisterCommand(const ConCommand& cmd) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_commands.emplace(cmd.name, cmd);
}

void ConsoleSystem::UnregisterCommand(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_commands.erase(name);
}

ConCommand* ConsoleSystem::FindCommand(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    auto it = m_commands.find(name);
    return it != m_commands.end() ? &it->second : nullptr;
}

void ConsoleSystem::SetAlias(const std::string& name, const std::string& commandLine) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_aliases[name] = commandLine;
}

void ConsoleSystem::RemoveAlias(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_aliases.erase(name);
}

bool ConsoleSystem::HasAlias(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_aliases.find(name) != m_aliases.end();
}

std::string ConsoleSystem::GetAlias(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    auto it = m_aliases.find(name);
    return it != m_aliases.end() ? it->second : "";
}

void ConsoleSystem::AddHistory(const std::string& line) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (!line.empty()) {
        // Don't add duplicates consecutively
        if (m_history.empty() || m_history.front() != line) {
            m_history.push_front(line);
            if (m_history.size() > 200) m_history.pop_back();
        }
    }
    m_historyNavigationIndex = -1;
}

const std::deque<std::string>& ConsoleSystem::GetHistory() {
    return m_history;
}

void ConsoleSystem::ClearHistory() {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_history.clear();
    m_historyNavigationIndex = -1;
}

std::string ConsoleSystem::SearchHistory(const std::string& prefix, bool reverse) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (m_history.empty()) return "";
    
    int startIndex = m_historyNavigationIndex;
    if (startIndex == -1) {
        startIndex = reverse ? static_cast<int>(m_history.size()) - 1 : 0;
    } else {
        startIndex += reverse ? -1 : 1;
    }
    
    // Search in the specified direction
    if (reverse) {
        for (int i = startIndex; i >= 0; --i) {
            if (m_history[i].find(prefix) == 0) {
                m_historyNavigationIndex = i;
                return m_history[i];
            }
        }
    } else {
        for (int i = startIndex; i < m_history.size(); ++i) {
            if (m_history[i].find(prefix) == 0) {
                m_historyNavigationIndex = i;
                return m_history[i];
            }
        }
    }
    
    return "";
}

size_t ConsoleSystem::AddListener(const std::string& cmdName, CommandCallback cb) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    size_t id = m_nextListenerId++;
    m_listeners[cmdName][id] = cb;
    return id;
}

void ConsoleSystem::RemoveListener(const std::string& cmdName, size_t listenerId) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    auto it = m_listeners.find(cmdName);
    if (it != m_listeners.end()) {
        it->second.erase(listenerId);
        if (it->second.empty()) m_listeners.erase(it);
    }
}

std::vector<std::string> ConsoleSystem::AutoComplete(const std::string& prefix, size_t maxResults) {
    std::vector<std::string> out;
    
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        
        // Commands
        for (const auto& kv : m_commands) {
            if (kv.first.rfind(prefix, 0) == 0 && !(kv.second.flags & CMD_HIDDEN)) {
                out.push_back(kv.first);
                if (out.size() >= maxResults) break;
            }
        }
        if (out.size() >= maxResults) return out;
        
        // Aliases
        for (const auto& kv : m_aliases) {
            if (kv.first.rfind(prefix, 0) == 0) {
                out.push_back(kv.first);
                if (out.size() >= maxResults) break;
            }
        }
        if (out.size() >= maxResults) return out;
    }
    
    // CVars
    auto decls = CVarRegistry::Instance().GetDeclarations();
    for (const auto& d : decls) {
        if (d.name.rfind(prefix, 0) == 0) {
            out.push_back(d.name);
            if (out.size() >= maxResults) break;
        }
    }
    
    return out;
}

std::vector<ConsoleSystem::Suggestion> ConsoleSystem::GetSuggestions(const std::string& prefix) {
    std::vector<Suggestion> suggestions;
    
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        
        // Commands
        for (const auto& kv : m_commands) {
            if (kv.first.rfind(prefix, 0) == 0 && !(kv.second.flags & CMD_HIDDEN)) {
                suggestions.push_back({kv.first, "command", kv.second.description});
            }
        }
        
        // Aliases
        for (const auto& kv : m_aliases) {
            if (kv.first.rfind(prefix, 0) == 0) {
                suggestions.push_back({kv.first, "alias", "Alias for: " + kv.second});
            }
        }
    }
    
    // CVars
    auto decls = CVarRegistry::Instance().GetDeclarations();
    for (const auto& d : decls) {
        if (d.name.rfind(prefix, 0) == 0) {
            suggestions.push_back({d.name, "cvar", d.description});
        }
    }
    
    // Sort by type then name
    std::sort(suggestions.begin(), suggestions.end(), [](const Suggestion& a, const Suggestion& b) {
        if (a.type != b.type) return a.type < b.type;
        return a.name < b.name;
    });
    
    return suggestions;
}

void ConsoleSystem::Print(const std::string& message) {
    if (m_outputCallback) {
        m_outputCallback(message);
    } else {
        LOG_INFO("Console", "{}", message);
    }
}

std::string ConsoleSystem::ExecuteWithResult(const std::string& line) {
    std::stringstream result;
    auto oldCallback = m_outputCallback;
    
    m_outputCallback = [&result](const std::string& msg) {
        result << msg << "\n";
    };
    
    Execute(line);
    m_outputCallback = oldCallback;
    
    return result.str();
}

void ConsoleSystem::ExecuteFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        Print("Could not open file: " + filename);
        return;
    }
    
    std::string line;
    int lineNum = 0;
    int executedCount = 0;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == '/') continue;
        
        // Remove trailing whitespace
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (!line.empty()) {
            Execute(line);
            executedCount++;
        }
    }
    
    file.close();
    Print("Executed " + std::to_string(executedCount) + " commands from " + filename);
}

static inline std::vector<std::string> TokenizeQuoted(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    bool escapeNext = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        
        if (escapeNext) {
            current.push_back(c);
            escapeNext = false;
            continue;
        }
        
        if (c == '\\') {
            escapeNext = true;
            continue;
        }
        
        if (c == '"') {
            inQuotes = !inQuotes;
            continue;
        }

        if (!inQuotes && std::isspace(static_cast<unsigned char>(c))) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(c);
        }
    }

    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

void ConsoleSystem::ExecuteSingle(const std::string& singleLine) {
    AddHistory(singleLine);
    ExecuteSingle(singleLine, 0);
}

void ConsoleSystem::ExecuteSingle(const std::string& singleLine, int depth) {
    if (singleLine.empty()) return;

    // защита от рекурсий
    if (depth > 16) {
        Print("Alias recursion limit reached (>16)");
        return;
    }

    auto tokens = TokenizeQuoted(singleLine);
    if (tokens.empty()) return;

    std::string name = tokens[0];
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        auto ait = m_aliases.find(name);
        if (ait != m_aliases.end()) {
            ExecuteSingle(ait->second, depth + 1);
            return;
        }
    }
    
    ConCommand* cmd = FindCommand(name);
    if (cmd) {
        // Check command flags
        if ((cmd->flags & CMD_CHEAT) && !GET_CVAR(bool, "sv_cheats")) {
            Print("Command '" + name + "' requires cheats to be enabled");
            return;
        }
        
        if (cmd->callback) {
            try {
                cmd->callback(args);
            } catch (const std::exception& e) {
                Print("Exception in command '" + name + "': " + e.what());
            }
        }

        std::unordered_map<size_t, CommandCallback> listenersCopy;
        {
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            auto it = m_listeners.find(name);
            if (it != m_listeners.end()) listenersCopy = it->second;
        }
        for (auto& kv : listenersCopy) {
            try { kv.second(args); } catch (...) {}
        }
        return;
    }

    // --- CVars ---
    if (auto* var = Engine::Application::Get()->GetCVar().Find(name)) {
        if (args.empty()) {
            // Get CVar value
            std::string value = Engine::Application::Get()->GetCVar().GetAsString(name);
            std::string description = Engine::Application::Get()->GetCVar().GetDescription(name);
            Print(name + " = \"" + value + "\" - " + description);
        } else {
            // Set CVar value
            std::string valueStr = args[0];
            for (size_t i = 1; i < args.size(); ++i) valueStr += " " + args[i];
            
            if (Engine::Application::Get()->GetCVar().SetFromString(name, valueStr, CVarSetSource::Console)) {
                std::string newValue = Engine::Application::Get()->GetCVar().GetAsString(name);
                Print(name + " = \"" + newValue + "\"");
            }
        }
        return;
    }

    Print("Unknown command or cvar: " + name);
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