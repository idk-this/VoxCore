// ConsoleUI.h
#pragma once

#include <deque>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <sstream>
#include <ostream>
#include <algorithm>
#include <functional>

#include "Core/CVar/Console.h"
#include "Core/Export.h"
#include "Core/Log/Logger.h"
#include "libs/imgui/imgui.h"

class Logger;

struct ColoredSegment {
    std::string text;
    ImVec4 color;
    bool isLink = false;
    std::string linkTarget;
};

struct ConsoleLine {
    std::vector<ColoredSegment> segments;
    bool selected = false;
    double timestamp = 0.0;
    std::string source; // "console", "log", "system"
};

class ConsoleStreamBuf : public std::streambuf {
public:
    ConsoleStreamBuf(std::deque<ConsoleLine>& items, std::mutex& mtx,
                    std::function<void(const std::string&)> linkCallback = nullptr)
        : m_items(items), m_mutex(mtx), m_linkCallback(linkCallback)
    {
        setp(m_buffer, m_buffer + sizeof(m_buffer) - 1);
    }

    void SetLinkCallback(std::function<void(const std::string&)> callback) {
        m_linkCallback = callback;
    }
    [[nodiscard]] std::function<void(const std::string&)> GetLinkCallback() const { return m_linkCallback; }

protected:
    int overflow(int ch) override {
        if (ch != EOF) {
            *pptr() = static_cast<char>(ch);
            pbump(1);
        }
        return sync();
    }

    int sync() override {
        if (pbase() != pptr()) {
            std::string msg(pbase(), pptr());
            ProcessMessage(msg);
            pbump(static_cast<int>(pbase() - pptr()));
            setp(m_buffer, m_buffer + sizeof(m_buffer) - 1);
        }
        return 0;
    }

private:
    void ProcessMessage(const std::string& msg) {
        std::vector<ColoredSegment> segments;
        size_t pos = 0;

        // Parse ANSI color codes and custom formatting
        while (pos < msg.size()) {
            if (msg[pos] == '\033') {
                // ANSI escape sequence
                auto m_pos = msg.find('m', pos);
                if (m_pos != std::string::npos) {
                    std::string code = msg.substr(pos, m_pos - pos + 1);
                    ImVec4 color = ParseANSIColor(code);
                    size_t start = m_pos + 1;
                    size_t next = msg.find("\033", start);
                    if (next == std::string::npos) next = msg.size();
                    std::string text = msg.substr(start, next - start);
                    segments.push_back({ text, color });
                    pos = next;
                    continue;
                }
            } else if (msg[pos] == '[' && m_linkCallback) {
                // Potential link: [link text](target)
                auto end_bracket = msg.find(']', pos);
                if (end_bracket != std::string::npos &&
                    end_bracket + 1 < msg.size() &&
                    msg[end_bracket + 1] == '(') {
                    auto end_paren = msg.find(')', end_bracket + 2);
                    if (end_paren != std::string::npos) {
                        std::string linkText = msg.substr(pos + 1, end_bracket - pos - 1);
                        std::string target = msg.substr(end_bracket + 2, end_paren - end_bracket - 2);
                        segments.push_back({ linkText, ImVec4(0.4f, 0.7f, 1.0f, 1.0f), true, target });
                        pos = end_paren + 1;
                        continue;
                    }
                }
            }

            size_t next = msg.find_first_of("\033[", pos);
            if (next == std::string::npos) next = msg.size();
            std::string text = msg.substr(pos, next - pos);
            if (!text.empty()) {
                segments.push_back({ text, ImVec4(1,1,1,1) });
            }
            pos = next;
        }

        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double>(now.time_since_epoch());

        std::lock_guard<std::mutex> lock(m_mutex);
        m_items.push_back({ segments, false, duration.count(), "log" });

        // Limit console size
        if (m_items.size() > 1000) {
            m_items.pop_front();
        }
    }

    ImVec4 ParseANSIColor(const std::string& code) {
        if (code.find("[31m") != std::string::npos) return ImVec4(1.0f, 0.4f, 0.4f, 1.0f); // Red
        if (code.find("[32m") != std::string::npos) return ImVec4(0.5f, 1.0f, 0.5f, 1.0f); // Green
        if (code.find("[33m") != std::string::npos) return ImVec4(1.0f, 1.0f, 0.5f, 1.0f); // Yellow
        if (code.find("[34m") != std::string::npos) return ImVec4(0.5f, 0.8f, 1.0f, 1.0f); // Blue
        if (code.find("[35m") != std::string::npos) return ImVec4(0.8f, 0.5f, 1.0f, 1.0f); // Magenta
        if (code.find("[36m") != std::string::npos) return ImVec4(0.5f, 1.0f, 1.0f, 1.0f); // Cyan
        if (code.find("[90m") != std::string::npos) return ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray
        return ImVec4(1,1,1,1); // Default white
    }

    std::deque<ConsoleLine>& m_items;
    std::mutex& m_mutex;
    std::function<void(const std::string&)> m_linkCallback;
    char m_buffer[1024];
};

class VOXCORE_API ConsoleUI {
public:
    ConsoleUI();
    ~ConsoleUI() = default;

    void InitializeLoggerHook(Logger* logger);
    void SetVisible(bool visible) { m_open = visible; }
    bool IsVisible() const { return m_open; }
    void Toggle() { m_open = !m_open; }

    void AddLog(const std::string& text, const ImVec4& color = ImVec4(1,1,1,1),
                const std::string& source = "console");
    void AddLogWithTimestamp(const std::string& text, const ImVec4& color = ImVec4(1,1,1,1));

    void Draw(const char* title = "Console");

    void Clear();
    void CopySelected();
    void CopyAll();

    // Auto-complete
    std::vector<std::string> GetAutoCompleteSuggestions(const std::string& input);
    void SetAutoCompleteEnabled(bool enabled) { m_autoCompleteEnabled = enabled; }

private:
    void DrawLogArea();
    void DrawInputArea();
    void DrawAutoComplete();
    void DrawAutoCompleteItems();
    void HandleLineClick(size_t clickedIndex, bool ctrlDown, bool shiftDown);
    void ClearSelection();

    static int InputCallback(ImGuiInputTextCallbackData* data);

    void NavigateHistory(int direction);
    void ResetHistoryNavigation();

    bool m_open = true;
    char m_inputBuf[512] = {};
    std::deque<ConsoleLine> m_items;
    std::vector<std::string> m_history;
    int m_lastSelectedIndex = -1;
    bool m_scrollToBottom = true;
    bool m_autoScroll = true;
    bool m_autoCompleteEnabled = true;


    std::vector<ConsoleSystem::Suggestion> m_suggestions;
    int m_selectedSuggestion = -1;
    bool m_showSuggestions = false;

    int m_historyIndex = -1;
    std::string m_currentInput;

    std::mutex m_mutex;
    std::unique_ptr<ConsoleStreamBuf> m_streamBuf;
    std::unique_ptr<std::ostream> m_consoleStream;

    float m_alpha = 0.9f;
};