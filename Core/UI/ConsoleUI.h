//
// Created by IDKTHIS on 05.10.2025.
//

#pragma once

#include <deque>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <sstream>
#include <ostream>
#include <algorithm>

#include "Core/CVar/Console.h"
#include "Core/Export.h"
#include "Core/Log/Logger.h"
#include "libs/imgui/imgui.h"

class Logger;

struct ColoredSegment {
    std::string text;
    ImVec4 color;
};

struct ConsoleLine {
    std::vector<ColoredSegment> segments;
    bool selected = false;
};

class ConsoleStreamBuf : public std::streambuf {
public:
    ConsoleStreamBuf(std::deque<ConsoleLine>& items, std::mutex& mtx)
        : m_items(items), m_mutex(mtx)
    {
        setp(m_buffer, m_buffer + sizeof(m_buffer) - 1);
    }

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

            std::vector<ColoredSegment> segments;
            size_t pos = 0;
            while (pos < msg.size()) {
                if (msg[pos] == '\033') {
                    auto m_pos = msg.find("m", pos);
                    if (m_pos != std::string::npos) {
                        std::string code = msg.substr(pos, m_pos - pos + 1);
                        ImVec4 color = ParseANSIColor(code);
                        size_t start = m_pos + 1;
                        size_t next = msg.find("\033", start);
                        std::string text = msg.substr(start, next - start);
                        segments.push_back({ text, color });
                        pos = next;
                        continue;
                    }
                }
                size_t next = msg.find('\033', pos);
                std::string text = msg.substr(pos, next - pos);
                segments.push_back({ text, ImVec4(1,1,1,1) });
                pos = next;
            }

            std::lock_guard<std::mutex> lock(m_mutex);
            m_items.push_back({ segments, false });

            pbump(static_cast<int>(pbase() - pptr()));
            setp(m_buffer, m_buffer + sizeof(m_buffer) - 1);
        }
        return 0;
    }

private:
    ImVec4 ParseANSIColor(const std::string& code) {
        if (code.find("[31m") != std::string::npos) return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        if (code.find("[32m") != std::string::npos) return ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
        if (code.find("[33m") != std::string::npos) return ImVec4(1.0f, 1.0f, 0.5f, 1.0f);
        if (code.find("[35m") != std::string::npos) return ImVec4(0.8f, 0.5f, 1.0f, 1.0f);
        return ImVec4(1,1,1,1);
    }

    std::deque<ConsoleLine>& m_items;
    std::mutex& m_mutex;
    char m_buffer[512];
};

class VOXCORE_API ConsoleUI {
public:
    ConsoleUI() = default;
    ~ConsoleUI() = default;

    void InitializeLoggerHook(Logger* logger) {
        m_streamBuf = std::make_unique<ConsoleStreamBuf>(m_items, m_mutex);
        m_consoleStream = std::make_unique<std::ostream>(m_streamBuf.get());
        logger->add_output("*", *m_consoleStream);
    }

    void AddLog(const std::string& text, const ImVec4& color = ImVec4(1,1,1,1)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_items.push_back({ { { text, color } }, false });
        m_scrollToBottom = true;
    }

    void Draw(const char* title = "Console") {
        if (!m_open) return;

        ImGui::SetNextWindowSize(ImVec2(700,400), ImGuiCond_FirstUseEver);
        ImGui::Begin(title, &m_open);

        DrawLogArea();
        DrawInputArea();

        ImGui::End();
    }

private:
    void DrawLogArea() {
        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(0.26f, 0.59f, 0.98f, 0.35f));

        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0,-ImGui::GetFrameHeightWithSpacing()), false,
                             ImGuiWindowFlags_HorizontalScrollbar)) {

            std::lock_guard<std::mutex> lock(m_mutex);

            if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
                ClearSelection();
            }

            for (size_t i = 0; i < m_items.size(); i++) {
                auto& line = m_items[i];

                std::string fullLine;
                for (const auto& seg : line.segments) {
                    fullLine += seg.text;
                }

                ImGui::PushID(static_cast<int>(i));

                ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                ImVec2 lineSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight());

                if (line.selected) {
                    ImGui::GetWindowDrawList()->AddRectFilled(
                        cursorPos,
                        ImVec2(cursorPos.x + lineSize.x, cursorPos.y + lineSize.y),
                        ImGui::GetColorU32(ImGuiCol_TextSelectedBg)
                    );
                }

                ImGui::InvisibleButton("##line_button", lineSize);
                bool isHovered = ImGui::IsItemHovered();

                if (isHovered && ImGui::IsMouseClicked(0)) {
                    HandleLineClick(i, ImGui::GetIO().KeyCtrl, ImGui::GetIO().KeyShift);
                }

                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Copy Selected")) {
                        CopySelectedLines();
                    }
                    if (ImGui::MenuItem("Copy All")) {
                        CopyAllLines();
                    }
                    ImGui::EndPopup();
                }

                ImGui::SetCursorScreenPos(cursorPos);
                for (const auto& seg : line.segments) {
                    ImGui::PushStyleColor(ImGuiCol_Text, seg.color);
                    ImGui::TextUnformatted(seg.text.c_str());
                    ImGui::PopStyleColor();
                    ImGui::SameLine(0,0);
                }
                ImGui::NewLine();

                ImGui::PopID();
            }

            if (m_scrollToBottom) {
                ImGui::SetScrollHereY(1.0f);
                m_scrollToBottom = false;
            }
        }
        ImGui::EndChild();

        ImGui::PopStyleColor();
    }

    void DrawInputArea() {
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##ConsoleInput", m_inputBuf, IM_ARRAYSIZE(m_inputBuf),
                             ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory,
                             InputCallback, this)) {
            std::string line(m_inputBuf);
            if (!line.empty()) {
                AddLog("> " + line);
                ConsoleSystem::Instance().Execute(line);
            }
            m_inputBuf[0] = '\0';
        }
        ImGui::PopItemWidth();
    }

    void HandleLineClick(size_t clickedIndex, bool ctrlDown, bool shiftDown) {
        if (shiftDown && m_lastSelectedIndex != -1) {
            size_t start = std::min(static_cast<size_t>(m_lastSelectedIndex), clickedIndex);
            size_t end = std::max(static_cast<size_t>(m_lastSelectedIndex), clickedIndex);

            for (size_t i = start; i <= end; i++) {
                if (i < m_items.size()) {
                    m_items[i].selected = true;
                }
            }
        } else if (ctrlDown) {
            if (clickedIndex < m_items.size()) {
                m_items[clickedIndex].selected = !m_items[clickedIndex].selected;
                m_lastSelectedIndex = static_cast<int>(clickedIndex);
            }
        } else {
            ClearSelection();
            if (clickedIndex < m_items.size()) {
                m_items[clickedIndex].selected = true;
                m_lastSelectedIndex = static_cast<int>(clickedIndex);
            }
        }
    }

    void ClearSelection() {
        for (auto& line : m_items) {
            line.selected = false;
        }
        m_lastSelectedIndex = -1;
    }

    void CopySelectedLines() {
        std::string copyText;
        for (const auto& line : m_items) {
            if (line.selected) {
                for (const auto& seg : line.segments) {
                    copyText += seg.text;
                }
                copyText += "\n";
            }
        }
        if (!copyText.empty()) {
            ImGui::SetClipboardText(copyText.c_str());
        }
    }

    void CopyAllLines() {
        std::string copyText;
        for (const auto& line : m_items) {
            for (const auto& seg : line.segments) {
                copyText += seg.text;
            }
            copyText += "\n";
        }
        if (!copyText.empty()) {
            ImGui::SetClipboardText(copyText.c_str());
        }
    }

    static int InputCallback(ImGuiInputTextCallbackData* data) {
        auto* self = static_cast<ConsoleUI*>(data->UserData);
        if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
            // TODO: history navigation
        }
        return 0;
    }

private:
    bool m_open = true;
    char m_inputBuf[256] = {};
    std::deque<ConsoleLine> m_items;
    std::vector<std::string> m_commands;
    std::vector<std::string> m_history;
    int m_lastSelectedIndex = -1;
    bool m_scrollToBottom = true;

    std::mutex m_mutex;
    std::unique_ptr<ConsoleStreamBuf> m_streamBuf;
    std::unique_ptr<std::ostream> m_consoleStream;
};