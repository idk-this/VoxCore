//
// Created by IDKTHIS on 27.09.2025.
//

#pragma once
#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

class VulkanInstance;
class PhysicalDevice;
class LogicalDevice;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanCommandSystem;
class IShaderPipeline;
enum class PipelineType;


struct VulkanContext {
    std::unique_ptr<VulkanInstance> instance;
    std::unique_ptr<PhysicalDevice> physicalDevice;
    std::unique_ptr<LogicalDevice> logicalDevice;
    std::unique_ptr<VulkanSwapChain> swapchain;
    std::unique_ptr<VulkanRenderPass> renderPass;
    std::unordered_map<PipelineType, std::unique_ptr<IShaderPipeline>> pipelines;
    std::unique_ptr<VulkanCommandSystem> commandSystem;
    vk::SurfaceKHR surface = nullptr;
};
