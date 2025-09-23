//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once

#include <string>
#include <cstdint>
#include <atomic>

#define UCLASS(CLASS_TYPE) \
    virtual void InitClass() override { SetClassName(#CLASS_TYPE); }

class UObject {
public:
    virtual void InitClass() { SetClassName("UObject"); }
    UObject() : m_id(++s_nextID) {
        InitClass();
    }
    virtual ~UObject() = default;

    uint64_t GetID() const { return m_id; }

    virtual std::string GetName() const { return m_name; }
    virtual void SetName(const std::string& name) { m_name = name; }

protected:
    uint64_t m_id;
    std::string m_name = "UObject";
    std::string m_className;
protected:
    void SetClassName(const std::string& name) { m_className = name; }

private:
    inline static std::atomic<uint64_t> s_nextID = 0;
};