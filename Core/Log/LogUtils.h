#pragma once
#include "Logger.h"
#include <chrono>
#include <functional>
#include <thread>

namespace Engine::Core::Log {

    // Класс для автоматического логирования времени выполнения блока кода
    class ScopedTimer {
    public:
        ScopedTimer(std::string_view name, LogLevel level = LogLevel::Debug, 
                    const LogCategory& category = LOG_CATEGORY_DEFAULT)
            : name(name), level(level), category(category),
              start(std::chrono::high_resolution_clock::now()) {}

        ~ScopedTimer() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            Logger::Get().Logf(level, category, "{} completed in {:.3f} ms", 
                              name, duration / 1000.0f);
        }

    private:
        std::string_view name;
        LogLevel level;
        LogCategory category;
        std::chrono::high_resolution_clock::time_point start;
    };

    // Обертка для вызова функции с автоматическим логированием входа и выхода
    template<typename Func, typename... Args>
    auto LoggedCall(LogLevel level, const LogCategory& category, 
                    std::string_view functionName, Func&& func, Args&&... args) {
        auto threadId = std::this_thread::get_id();
        Logger::Get().Logf(level, category, "[Thread {:x}] Entering {}", 
                          std::hash<std::thread::id>{}(threadId), functionName);

        if constexpr (std::is_same_v<std::invoke_result_t<Func, Args...>, void>) {
            // Функция без возвращаемого значения
            func(std::forward<Args>(args)...);
            Logger::Get().Logf(level, category, "[Thread {:x}] Exiting {}", 
                              std::hash<std::thread::id>{}(threadId), functionName);
        } else {
            // Функция с возвращаемым значением
            auto result = func(std::forward<Args>(args)...);
            Logger::Get().Logf(level, category, "[Thread {:x}] Exiting {} with result", 
                              std::hash<std::thread::id>{}(threadId), functionName);
            return result;
        }
    }

    // Получение имени типа для логирования (с поддержкой pretty printing)
    template<typename T>
    constexpr std::string_view TypeName() {
#ifdef __clang__
        constexpr std::string_view prefix = "[T = ";
        constexpr std::string_view suffix = "]";
        constexpr std::string_view function = __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
        constexpr std::string_view prefix = "with T = ";
        constexpr std::string_view suffix = ";";
        constexpr std::string_view function = __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
        constexpr std::string_view prefix = "TypeName<";
        constexpr std::string_view suffix = ">(void)";
        constexpr std::string_view function = __FUNCSIG__;
#else
        // Fallback для неизвестных компиляторов
        return "unknown_type";
#endif

        constexpr auto start = function.find(prefix) + prefix.size();
        constexpr auto end = function.rfind(suffix);
        constexpr auto size = end - start;

        return function.substr(start, size);
    }
}


#define LOG_FUNCTION_TIMER() \
    ::Engine::Core::Log::ScopedTimer timer##__LINE__(__FUNCTION__)


#define LOG_TIMED_BLOCK(name) \
    ::Engine::Core::Log::ScopedTimer timer##__LINE__(name)


#define LOG_TYPE(variable) \
    LOGF_DEBUG("Type of {} is {}", #variable, ::Engine::Core::Log::TypeName<decltype(variable)>())
