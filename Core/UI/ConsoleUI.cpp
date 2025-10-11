// ConsoleUI.cpp
#include "ConsoleUI.h"
#include <chrono>
#include <iomanip>
#include <sstream>

#include "imgui_internal.h"

ConsoleUI::ConsoleUI() {
    // Initialize with default filters
    m_sourceFilters = {"console", "log", "system"};
}

void ConsoleUI::InitializeLoggerHook(Logger* logger) {
    m_streamBuf = std::make_unique<ConsoleStreamBuf>(m_items, m_mutex);
    m_consoleStream = std::make_unique<std::ostream>(m_streamBuf.get());
    logger->add_output("*", *m_consoleStream);

    // Set up console system output callback
    ConsoleSystem::Instance().SetOutputCallback([this](const std::string& msg) {
        this->AddLog(msg, ImVec4(1,1,1,1), "console");
    });
}

void ConsoleUI::AddLog(const std::string& text, const ImVec4& color, const std::string& source) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration<double>(now.time_since_epoch());

    m_items.push_back({ { { text, color } }, false, duration.count(), source });
    m_scrollToBottom = true;
    m_filterDirty = true;

    // Limit console size
    if (m_items.size() > 2000) {
        m_items.pop_front();
        m_filterDirty = true;
    }
}

void ConsoleUI::AddLogWithTimestamp(const std::string& text, const ImVec4& color) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] " << text;
    AddLog(ss.str(), color);
}

// Добавьте этот код в функцию Draw (после DrawInputArea())
void ConsoleUI::Draw(const char* title) {
    if (!m_open) return;

    ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);

    if (m_darkTheme) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, m_alpha));
    } else {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.9f, 0.9f, 0.9f, m_alpha));
    }

    if (ImGui::Begin(title, &m_open)) {
        DrawLogArea();
        DrawInputArea();

        // Обработка клавиш для автодополнения - только когда InputText активен
        bool isInputTextActive = ImGui::IsItemActive();

        if (isInputTextActive && m_showSuggestions && m_autoCompleteEnabled && !m_suggestions.empty()) {
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                m_showSuggestions = false;
            } else if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
                if (m_selectedSuggestion >= 0 && m_selectedSuggestion < m_suggestions.size()) {
                    strcpy_s(m_inputBuf, m_suggestions[m_selectedSuggestion].name.c_str());
                    m_showSuggestions = false;
                }
            } else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                if (m_selectedSuggestion > 0) {
                    m_selectedSuggestion--;
                } else {
                    m_selectedSuggestion = static_cast<int>(m_suggestions.size()) - 1;
                }
            } else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                if (m_selectedSuggestion < static_cast<int>(m_suggestions.size()) - 1) {
                    m_selectedSuggestion++;
                } else {
                    m_selectedSuggestion = 0;
                }
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();

    // Рисуем автодополнение ПОСЛЕ основного окна, чтобы оно было поверх
    if (m_showSuggestions && m_autoCompleteEnabled && !m_suggestions.empty()) {
        DrawAutoComplete();
    }
}

void ConsoleUI::DrawLogArea() {
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(0.26f, 0.59f, 0.98f, 0.35f));

    const float footer_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height), false,
                         ImGuiWindowFlags_HorizontalScrollbar)) {

        std::lock_guard<std::mutex> lock(m_mutex);

        // Apply filter if dirty
        if (m_filterDirty) {
            ApplyFilter();
            m_filterDirty = false;
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
            ClearSelection();
        }

        // Display items
        const auto& displayItems = m_filterBuf[0] ? m_filteredIndices : [this]() {
            std::vector<size_t> indices(m_items.size());
            for (size_t i = 0; i < m_items.size(); ++i) indices[i] = i;
            return indices;
        }();

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(displayItems.size()));

        while (clipper.Step()) {
            for (int displayIdx = clipper.DisplayStart; displayIdx < clipper.DisplayEnd; displayIdx++) {
                if (displayIdx >= displayItems.size()) continue;

                size_t itemIdx = displayItems[displayIdx];
                if (itemIdx >= m_items.size()) continue;

                auto& line = m_items[itemIdx];

                if (!IsLineFiltered(line)) continue;

                ImGui::PushID(static_cast<int>(itemIdx));

                // Timestamp
                std::string prefix;
                if (m_showTimestamps) {
                    std::stringstream ss;
                    ss << "[" << std::fixed << std::setprecision(3) << line.timestamp << "] ";
                    prefix = ss.str();
                }

                ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                float lineHeight = ImGui::GetTextLineHeight();
                ImVec2 lineSize = ImVec2(ImGui::GetContentRegionAvail().x, lineHeight);

                // Selection background
                if (line.selected) {
                    ImGui::GetWindowDrawList()->AddRectFilled(
                        cursorPos,
                        ImVec2(cursorPos.x + lineSize.x, cursorPos.y + lineHeight),
                        ImGui::GetColorU32(ImGuiCol_TextSelectedBg)
                    );
                }

                // Invisible button for selection
                ImGui::InvisibleButton("##line_button", lineSize);
                bool isHovered = ImGui::IsItemHovered();

                if (isHovered && ImGui::IsMouseClicked(0)) {
                    HandleLineClick(itemIdx, ImGui::GetIO().KeyCtrl, ImGui::GetIO().KeyShift);
                }

                // Context menu
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Copy Line")) {
                        std::string copyText;
                        for (const auto& seg : line.segments) copyText += seg.text;
                        ImGui::SetClipboardText(copyText.c_str());
                    }
                    if (ImGui::MenuItem("Copy Selected")) {
                        CopySelected();
                    }
                    if (ImGui::MenuItem("Copy All")) {
                        CopyAll();
                    }
                    if (ImGui::MenuItem("Clear")) {
                        Clear();
                    }
                    ImGui::EndPopup();
                }

                // Draw text
                ImGui::SetCursorScreenPos(cursorPos);

                // Timestamp
                if (!prefix.empty()) {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", prefix.c_str());
                    ImGui::SameLine(0, 0);
                }

                // Segments
                for (const auto& seg : line.segments) {
                    if (seg.isLink) {
                        ImGui::PushStyleColor(ImGuiCol_Text, seg.color);
                        ImGui::TextUnformatted(seg.text.c_str());
                        if (ImGui::IsItemClicked()) {
                            // Handle link click
                            if (m_streamBuf && m_streamBuf->GetLinkCallback()) {
                                m_streamBuf->GetLinkCallback()(seg.linkTarget);
                            }
                        }
                        ImGui::PopStyleColor();
                        ImGui::SameLine(0, 0);
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Text, seg.color);
                        ImGui::TextUnformatted(seg.text.c_str());
                        ImGui::PopStyleColor();
                        ImGui::SameLine(0, 0);
                    }
                }
                ImGui::NewLine();

                ImGui::PopID();
            }
        }

        if (m_scrollToBottom && m_autoScroll) {
            ImGui::SetScrollHereY(1.0f);
            m_scrollToBottom = false;
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleColor();
}

void ConsoleUI::DrawInputArea() {
    ImGui::PushItemWidth(-1);

    bool reclaimFocus = false;
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue |
                               ImGuiInputTextFlags_CallbackHistory |
                               ImGuiInputTextFlags_CallbackCompletion |
                               ImGuiInputTextFlags_CallbackAlways;

    if (ImGui::InputText("##ConsoleInput", m_inputBuf, IM_ARRAYSIZE(m_inputBuf), flags,
                        InputCallback, this)) {
        std::string line(m_inputBuf);
        if (!line.empty()) {
            AddLog("> " + line, ImVec4(1, 1, 0.5f, 1), "console");
            ConsoleSystem::Instance().Execute(line);

            // Add to history
            m_history.push_back(line);
            if (m_history.size() > 100) {
                m_history.erase(m_history.begin());
            }
        }
        strcpy_s(m_inputBuf, "");
        reclaimFocus = true;
        m_showSuggestions = false;
        ResetHistoryNavigation();
    }

    // Auto-focus on window appearing
    ImGui::SetItemDefaultFocus();
    if (reclaimFocus) {
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::PopItemWidth();
}

// ConsoleUI.cpp - исправленные функции DrawAutoComplete и InputCallback
void ConsoleUI::DrawAutoComplete() {
    if (m_suggestions.empty() || !m_showSuggestions) return;

    // Получаем позицию InputText в абсолютных координатах экрана
    ImVec2 pos = ImGui::GetMainViewport()->Pos;
    ImVec2 consolePos = ImGui::FindWindowByName("Console")->Pos;
    ImVec2 consoleSize = ImGui::FindWindowByName("Console")->Size;

    // Вычисляем позицию подсказок относительно окна консоли
    float inputTextY = consolePos.y + consoleSize.y - ImGui::GetFrameHeightWithSpacing() * 2 + 40;

    ImGui::SetNextWindowPos(ImVec2(consolePos.x, inputTextY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(consoleSize.x, 0)); // Высота будет автоматически рассчитана

    // Ключевые флаги: окно без фокуса, но поверх других
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_Tooltip; // Этот флаг гарантирует, что окно будет поверх

    // Принудительно устанавливаем окно поверх


    if (ImGui::Begin("##autocomplete", nullptr, flags)) {
        DrawAutoCompleteItems();

        // Дополнительная страховка - проверяем, активно ли окно подсказок
       // ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    }
    ImGui::End();
}




void ConsoleUI::DrawAutoCompleteItems() {
    for (int i = 0; i < m_suggestions.size(); ++i) {
        const auto& suggestion = m_suggestions[i];
        bool isSelected = (i == m_selectedSuggestion);

        ImGui::PushID(i);

        // Выделение при наведении
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
        }

        // Создаем невидимую кнопку для обработки кликов
        ImGui::Selectable("", isSelected, ImGuiSelectableFlags_AllowDoubleClick);

        // Обработка кликов на подсказках
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
            m_selectedSuggestion = i;
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            strcpy_s(m_inputBuf, suggestion.name.c_str());
            m_showSuggestions = false;
        }

        if (isSelected) {
            ImGui::PopStyleColor(3);
        }

        // Текст подсказки
        ImGui::SameLine();

        ImVec4 color = ImVec4(1, 1, 1, 1);
        if (suggestion.type == "command") color = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
        else if (suggestion.type == "cvar") color = ImVec4(0.5f, 0.8f, 1.0f, 1.0f);
        else if (suggestion.type == "alias") color = ImVec4(1.0f, 0.8f, 0.5f, 1.0f);

        ImGui::TextColored(color, "%s", suggestion.name.c_str());

        if (!suggestion.description.empty()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "- %s", suggestion.description.c_str());
        }

        ImGui::PopID();
    }
}

int ConsoleUI::InputCallback(ImGuiInputTextCallbackData* data) {
    auto* self = static_cast<ConsoleUI*>(data->UserData);

    switch (data->EventFlag) {
        case ImGuiInputTextFlags_CallbackHistory: {
            // Если есть активные подсказки - обрабатываем навигацию по ним
            if (self->m_showSuggestions && !self->m_suggestions.empty()) {
                if (data->EventKey == ImGuiKey_UpArrow) {
                    self->m_selectedSuggestion = (self->m_selectedSuggestion > 0)
                        ? self->m_selectedSuggestion - 1
                        : static_cast<int>(self->m_suggestions.size()) - 1;
                    return 1; // Запрещаем дальнейшую обработку
                } else if (data->EventKey == ImGuiKey_DownArrow) {
                    self->m_selectedSuggestion = (self->m_selectedSuggestion < static_cast<int>(self->m_suggestions.size()) - 1)
                        ? self->m_selectedSuggestion + 1
                        : 0;
                    return 1; // Запрещаем дальнейшую обработку
                }
            } else {
                // Навигация по истории, если автодополнение не активно
                if (data->EventKey == ImGuiKey_UpArrow) {
                    self->NavigateHistory(1);
                } else if (data->EventKey == ImGuiKey_DownArrow) {
                    self->NavigateHistory(-1);
                }
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, self->m_inputBuf);
            }
            break;
        }

        case ImGuiInputTextFlags_CallbackCompletion: {
            if (data->EventKey == ImGuiKey_Tab) {
                std::string currentText(data->Buf);
                auto suggestions = self->GetAutoCompleteSuggestions(currentText);
                if (!suggestions.empty()) {
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, suggestions[0].c_str());
                    self->m_showSuggestions = false;
                }
            }
            break;
        }

        case ImGuiInputTextFlags_CallbackAlways: {
            // Обновляем подсказки при любом изменении текста
            if (self->m_autoCompleteEnabled) {
                std::string currentText(data->Buf);
                self->m_suggestions = ConsoleSystem::Instance().GetSuggestions(currentText);

                self->m_showSuggestions = !self->m_suggestions.empty() && !currentText.empty();

                // Автоматически выбираем первую подсказку
                if (self->m_showSuggestions && self->m_selectedSuggestion == -1) {
                    self->m_selectedSuggestion = 0;
                }
            } else {
                self->m_showSuggestions = false;
            }
            break;
        }
    }

    return 0;
}

void ConsoleUI::HandleLineClick(size_t clickedIndex, bool ctrlDown, bool shiftDown) {
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

void ConsoleUI::ClearSelection() {
    for (auto& line : m_items) {
        line.selected = false;
    }
    m_lastSelectedIndex = -1;
}

void ConsoleUI::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_items.clear();
    m_filteredIndices.clear();
    m_filterDirty = false;
}

void ConsoleUI::CopySelected() {
    std::string copyText;
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& line : m_items) {
        if (line.selected) {
            if (m_showTimestamps) {
                std::stringstream ss;
                ss << "[" << std::fixed << std::setprecision(3) << line.timestamp << "] ";
                copyText += ss.str();
            }
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

void ConsoleUI::CopyAll() {
    std::string copyText;
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& line : m_items) {
        if (IsLineFiltered(line)) {
            if (m_showTimestamps) {
                std::stringstream ss;
                ss << "[" << std::fixed << std::setprecision(3) << line.timestamp << "] ";
                copyText += ss.str();
            }
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

void ConsoleUI::SetFilter(const std::string& filter) {
    strcpy_s(m_filterBuf, filter.c_str());
    m_filterDirty = true;
}

bool ConsoleUI::IsLineFiltered(const ConsoleLine& line) const {
    // Source filter
    if (!m_sourceFilters.empty() &&
        std::find(m_sourceFilters.begin(), m_sourceFilters.end(), line.source) == m_sourceFilters.end()) {
        return false;
    }

    // Text filter
    if (m_filterBuf[0]) {
        std::string fullText;
        for (const auto& seg : line.segments) fullText += seg.text;
        return fullText.find(m_filterBuf) != std::string::npos;
    }

    return true;
}

void ConsoleUI::ApplyFilter() {
    m_filteredIndices.clear();
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (IsLineFiltered(m_items[i])) {
            m_filteredIndices.push_back(i);
        }
    }
}

std::vector<std::string> ConsoleUI::GetAutoCompleteSuggestions(const std::string& input) {
    return ConsoleSystem::Instance().AutoComplete(input);
}

void ConsoleUI::NavigateHistory(int direction) {
    if (m_history.empty()) return;

    if (m_historyIndex == -1) {
        m_currentInput = m_inputBuf;
    }

    m_historyIndex += direction;

    if (m_historyIndex < 0) {
        m_historyIndex = -1;
        strcpy_s(m_inputBuf, m_currentInput.c_str());
    } else if (m_historyIndex >= m_history.size()) {
        m_historyIndex = m_history.size() - 1;
    } else {
        strcpy_s(m_inputBuf, m_history[m_historyIndex].c_str());
    }
}

void ConsoleUI::ResetHistoryNavigation() {
    m_historyIndex = -1;
    m_currentInput.clear();
}

