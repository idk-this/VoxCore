//
// Created by IDKTHIS on 10.10.2025.
//

#pragma once


#include <string>
#include <functional>
#include <unordered_map>

namespace UISystem {
    class DataContext {
    public:
        virtual ~DataContext() = default;
        virtual std::string GetProperty(const std::string& path) = 0;
        virtual void SetProperty(const std::string& path, const std::string& value) = 0;
        virtual void BindEvent(const std::string& eventName, std::function<void()> handler) = 0;
    };

    class SimpleDataContext : public DataContext {
    public:
        std::string GetProperty(const std::string& path) override {
            auto it = properties.find(path);
            return it != properties.end() ? it->second : "";
        }

        void SetProperty(const std::string& path, const std::string& value) override {
            properties[path] = value;
            // Уведомление об изменении
        }

        void BindEvent(const std::string& eventName, std::function<void()> handler) override {
            events[eventName] = handler;
        }

        std::function<void()> GetEvent(const std::string& eventName) {
            auto it = events.find(eventName);
            return it != events.end() ? it->second : nullptr;
        }

    private:
        std::unordered_map<std::string, std::string> properties;
        std::unordered_map<std::string, std::function<void()>> events;
    };
}