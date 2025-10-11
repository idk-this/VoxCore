//
// Created by IDKTHIS on 10.10.2025.
//

#pragma once


#include <string>
#include <functional>
#include <unordered_map>

namespace UISystem {
    class DataObserver {
    public:
        virtual ~DataObserver() = default;
        virtual void OnDataChanged(const std::string& path, const std::string& value) = 0;
    };

    class DataContext {
    public:
        virtual ~DataContext() = default;
        virtual std::string GetProperty(const std::string& path) = 0;
        virtual void SetProperty(const std::string& path, const std::string& value) = 0;
        virtual void BindEvent(const std::string& eventName, std::function<void()> handler) = 0;

        // Добавляем методы для подписки на изменения
        virtual void AddObserver(const std::string& path, DataObserver* observer) = 0;
        virtual void RemoveObserver(const std::string& path, DataObserver* observer) = 0;
    };

    class SimpleDataContext : public DataContext {
    public:
        std::string GetProperty(const std::string& path) override {
            auto it = properties.find(path);
            return it != properties.end() ? it->second : "";
        }

        void SetProperty(const std::string& path, const std::string& value) override {
            properties[path] = value;
            // Уведомляем всех наблюдателей об изменении
            auto it = observers.find(path);
            if (it != observers.end()) {
                for (auto* observer : it->second) {
                    observer->OnDataChanged(path, value);
                }
            }
        }

        void BindEvent(const std::string& eventName, std::function<void()> handler) override {
            events[eventName] = handler;
        }

        void AddObserver(const std::string& path, DataObserver* observer) override {
            observers[path].push_back(observer);
        }

        void RemoveObserver(const std::string& path, DataObserver* observer) override {
            auto it = observers.find(path);
            if (it != observers.end()) {
                auto& vec = it->second;
                vec.erase(std::remove(vec.begin(), vec.end(), observer), vec.end());
            }
        }

        std::function<void()> GetEvent(const std::string& eventName) {
            auto it = events.find(eventName);
            return it != events.end() ? it->second : nullptr;
        }

    private:
        std::unordered_map<std::string, std::string> properties;
        std::unordered_map<std::string, std::function<void()>> events;
        std::unordered_map<std::string, std::vector<DataObserver*>> observers;
    };
}