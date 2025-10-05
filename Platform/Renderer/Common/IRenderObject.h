//
// Created by IDKTHIS on 29.09.2025.
//

#pragma once
#include <any>
#include <cstdint>
#include <vector>

class AActor;

class IRenderObject {
public:
    virtual ~IRenderObject() = default;

    virtual bool Initialize() = 0;
    virtual bool Draw(const std::any& drawInfo) = 0;
    virtual void Cleanup() = 0;

    virtual std::uint32_t GetIndexCount() const = 0;
    virtual bool IsValid() const = 0;
};
