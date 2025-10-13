//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once

#include <string>
#include <cstdint>
#include <atomic>
#include <regex>
#include <typeinfo>
#include "Core/Export.h"

class UObject {
public:
    virtual void InitClass() { SetClassName("UObject"); }
    UObject() : m_id(++s_nextID) { InitClass(); }
    virtual ~UObject() = default;

    uint64_t GetID() const { return m_id; }

    std::string GetName() const {
        return m_className + "_" + std::to_string(m_id);
    }

    const std::string& GetClassName() const { return m_className; }

protected:
    uint64_t m_id;
    std::string m_className;

protected:
    void SetClassName(const std::string& name) { m_className = name; }
    std::string GetCleanClassName() const {
        std::string name = typeid(*this).name();
        static const std::regex re("^(class |struct )?(.*::)?");
        return std::regex_replace(name, re, "");
    }

private:
    inline static std::atomic<uint64_t> s_nextID = 0;
    virtual const char* GetStaticClassName() const { return "UObject"; }
};

#define UCLASS(CLASS_NAME)                  \
public:                                     \
void InitClass() override { SetClassName(#CLASS_NAME); } \
static constexpr const char* StaticClassName() { return #CLASS_NAME; } \
virtual const char* GetStaticClassName() const override { return #CLASS_NAME; }