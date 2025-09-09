//
// Created by IDKTHIS on 05.09.2025.
//

#pragma once
#include "Platform/Renderer/Common/ICameraUBO.h"


/*
class VulkanCameraUBO : public ICameraUBO {
public:
    VulkanCameraUBO(vk::Device device,
                    vk::DescriptorSetLayout layout,
                    vk::DescriptorPool pool);

    void Update(const CameraData& data) override;
    void* GetHandle() const override { return (void*)&m_descriptorSet; }

private:
    VulkanBuffer m_buffer; // твой wrapper
    vk::DescriptorSet m_descriptorSet;
    vk::Device m_device;
};
*/
