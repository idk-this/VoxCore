//
// Created by IDKTHIS on 29.09.2025.
//

#pragma once
#include <memory>
#include <vector>

#include "ICameraUBO.h"
#include "IRenderObject.h"


class AActor;
class UMeshComponent;

class IRenderResourceManager {
public:
    virtual ~IRenderResourceManager() = default;

    virtual bool Initialize() = 0;
    virtual void Cleanup() = 0;

    /*virtual std::shared_ptr<IRenderObject> GetOrCreateRenderObject(
        UMeshComponent* mesh, const std::vector<AActor*>& actors) = 0;
    virtual void UpdateCameraData(void* cmd, const CameraData& cameraData) = 0;
    virtual std::any GetCameraBindData() const = 0;

    virtual void CleanupMeshResources(UMeshComponent* mesh) = 0;*/
};
