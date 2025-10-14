//
// Created by IDKTHIS on 13.10.2025.
//

#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Core/Export.h"
#include <cmath>

class VOXCORE_API CameraUtils {
public:
    static bool WorldToScreen(
        glm::mat4 view,glm::mat4 proj ,
        const glm::vec3& worldPos,
        glm::vec2& screenPos,
        int screenWidth = 1920,
        int screenHeight = 1080)
    {
        proj[1][1] *= -1;
        glm::vec4 clip = proj * view * glm::vec4(worldPos, 1.0f);

        glm::vec3 ndc = glm::vec3(clip) / clip.w;

        screenPos.x = (ndc.x * 0.5f + 0.5f) * screenWidth;
        screenPos.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * screenHeight;
        return true;
    }
};
