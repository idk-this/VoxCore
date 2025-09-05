//
// Created by IDKTHIS on 02.07.2025.
//

#include "VulkanRenderer.h"

#include <cassert>
#include <stdexcept>
#include <SDL3/SDL_vulkan.h>

#include "Core/CVar/CVar.h"
#include "Core/Log/Logger.h"
#include "Commands/VulkanCommandSystem.h"
#include "Devices/LogicalDevice.h"
#include "Devices/PhysicalDevice.h"
#include "Pipeline/GraphicsPipeline.h"
#include "RenderPass/VulkanRenderPass.h"
#include "Swapchain/VulkanSwapChain.h"
#include "Camera.h"
#include "Core/ECS/BaseClasses/UWorld.h"
#include "Core/ECS/Components/UTransformComponent.h"
#include "Core/ECS/Components/UMeshComponent.h"
#include <unordered_map>

CONVAR("r_max_frames_in_flight", 2, "Maximum number of frames in flight for swapchain", CVAR_RUNTIME_ONLY);


VulkanRenderer::VulkanRenderer() {
	m_instance = std::make_unique<VulkanInstance>();
	m_physicalDevice = std::make_unique<PhysicalDevice>(m_instance.get());
	m_logicalDevice = std::make_unique<LogicalDevice>(m_instance.get());
	m_swapchain = std::make_unique<VulkanSwapChain>(m_instance.get(), m_physicalDevice.get(), m_logicalDevice.get());
	m_renderPass = std::make_unique<VulkanRenderPass>(m_instance.get(), m_logicalDevice.get(), m_swapchain.get());
	m_graphicsPipeline = std::make_unique<GraphicsPipeline>( m_logicalDevice.get(), m_physicalDevice.get(), m_renderPass.get(), m_swapchain.get());
	m_commandSystem = std::make_unique<VulkanCommandSystem>(m_instance.get(), m_logicalDevice.get(), m_renderPass.get(), m_swapchain.get(), m_graphicsPipeline.get());
	// Инициализация камеры: позиция, yaw, pitch, fov, aspect, near, far
	m_camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 1.0f), 180.0f, 0.0f, 60.0f, 16.0f/9.0f, 0.1f, 100.0f);
};
VulkanRenderer::~VulkanRenderer() {
	//Cleanup();
};
VulkanShader* vulkanShader = nullptr;;
bool VulkanRenderer::Init(IWindow *window, UWorld* world) {
	if (!m_instance->Init()) {
		return  false;
	}
	VkSurfaceKHR c_surface;
	SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(window->GetNativeHandle()), m_instance->GetInstance(), nullptr, &c_surface); //TODO: Make CreateSurface in IWindow.
	m_surface = c_surface;
	vk::Extent2D windowExtent{ window->GetWidth(), window->GetHeight() };
	bool isSuccess =
		   m_physicalDevice->Init(m_surface) &&
		   m_logicalDevice->Init(m_physicalDevice.get()) &&
		   m_swapchain->Init(m_surface, windowExtent) &&
		   m_renderPass->Init();
	if (!isSuccess) {
		LOG_FATAL("Vulkan", "Failed to initialize Vulkan components.");
		return false;
	}
	vulkanShader = new VulkanShader(m_logicalDevice.get());
	vulkanShader->LoadFromFile("Engine/Assets/Shaders/BaseShader.glsl");

	m_graphicsPipeline->SetShader(vulkanShader);

	m_graphicsPipeline->Init();
	m_swapchain->CreateFramebuffers(m_renderPass.get());
	if (!m_commandSystem->Init(m_physicalDevice->GetGraphicsQueueFamilyIndex())) {
		return false;
	}
	m_imageAvailableSemaphores.resize(GET_CVAR(int, "r_max_frames_in_flight"));
	m_renderFinishedSemaphores.resize(m_swapchain->GetImageCount());
	m_inFlightFences.resize(GET_CVAR(int, "r_max_frames_in_flight"));
	for (int i = 0; i < GET_CVAR(int, "r_max_frames_in_flight"); ++i) {
		m_imageAvailableSemaphores[i] = m_logicalDevice->GetHandle().createSemaphore({});
		m_inFlightFences[i] = m_logicalDevice->GetHandle().createFence({vk::FenceCreateFlagBits::eSignaled});
	}
	for (uint32_t i = 0; i < m_swapchain->GetImageCount(); ++i) {
		m_renderFinishedSemaphores[i] = m_logicalDevice->GetHandle().createSemaphore({});
	}
	return true;
}

void VulkanRenderer::BeginFrame() {
	m_logicalDevice->GetHandle().waitForFences(m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	m_swapchain->BeginRender(m_imageAvailableSemaphores[m_currentFrame]);
	m_logicalDevice->GetHandle().resetFences(m_inFlightFences[m_currentFrame]);
	m_commandSystem->GetCommandBuffer(m_currentFrame).reset();
}

void VulkanRenderer::ProcessRender() {
	vk::CommandBufferBeginInfo beginInfo{};

	m_commandSystem->GetCommandBuffer(m_currentFrame).begin(&beginInfo);

	std::array<vk::ClearValue, 2> clearValues{};
	clearValues[0].color = {  0.1f, 0.1f, 0.1f, 1.0f  };
	clearValues[1].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

	vk::RenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.renderPass = m_renderPass->GetHandle();
	renderPassBeginInfo.framebuffer = m_swapchain->GetFramebuffer(m_swapchain->GetCurrentImageIndex());
	renderPassBeginInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_swapchain->GetSwapExtent();
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();
	m_commandSystem->GetCommandBuffer(m_currentFrame).beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	m_commandSystem->GetCommandBuffer(m_currentFrame).bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline->GetHandle());


	m_commandSystem->GetCommandBuffer(m_currentFrame).draw(96, 1, 0, 0);

	m_commandSystem->GetCommandBuffer(m_currentFrame).endRenderPass();
	m_commandSystem->GetCommandBuffer(m_currentFrame).end();
}

void VulkanRenderer::EndFrame() {
	std::array<vk::Semaphore, 1> waitSemaphores = { m_imageAvailableSemaphores[m_currentFrame] };
	std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	std::array<vk::Semaphore, 1> signalSemaphores = { m_renderFinishedSemaphores[m_swapchain->GetCurrentImageIndex()] };
	vk::SubmitInfo submitInfo{};
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());;
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.pCommandBuffers = &m_commandSystem->GetCommandBuffer(m_currentFrame);
	submitInfo.commandBufferCount = 1;
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	m_logicalDevice->GetGraphicsQueue().submit(submitInfo, m_inFlightFences[m_currentFrame]);
	m_swapchain->Present(m_renderFinishedSemaphores[m_swapchain->GetCurrentImageIndex()]);
	m_currentFrame = (m_currentFrame + 1) % GET_CVAR(int, "r_max_frames_in_flight");
}

void VulkanRenderer::Cleanup() {
	LOG_INFO("Vulkan", "Cleaning up Vulkan resources.");
	m_logicalDevice->GetHandle().waitIdle();

	m_commandSystem.reset();
	m_swapchain.reset();

	m_renderPass.reset();
	m_graphicsPipeline.reset();
	for (auto& semaphore : m_imageAvailableSemaphores) {
		if (semaphore) {
			m_logicalDevice->GetHandle().destroySemaphore(semaphore);
		}
	}
	m_imageAvailableSemaphores.clear();

	for (auto& semaphore : m_renderFinishedSemaphores) {
		if (semaphore) {
			m_logicalDevice->GetHandle().destroySemaphore(semaphore);
		}
	}
	m_renderFinishedSemaphores.clear();

	for (auto& fence : m_inFlightFences) {
		if (fence) {
			m_logicalDevice->GetHandle().destroyFence(fence);
		}
	}
	m_inFlightFences.clear();
	m_logicalDevice.reset();
	m_physicalDevice.reset();
	m_instance->GetInstance().destroySurfaceKHR(m_surface);
	m_instance.reset();
	LOG_INFO("Vulkan", "Vulkan resources cleaned up.");
}


void VulkanRenderer::RenderFrame() {
	BeginFrame();
	ProcessRender();
	EndFrame();
}
