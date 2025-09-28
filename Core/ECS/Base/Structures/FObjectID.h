//
// Created by IDKTHIS on 27.09.2025.
//

#pragma once
#include <cstdint>

struct FObjectID
{
    uint32_t index = 0;
    uint32_t generation = 0;

    bool operator==(const FObjectID& other) const {
        return index == other.index && generation == other.generation;
    }
    bool operator!=(const FObjectID& other) const {
        return !(*this == other);
    }
};