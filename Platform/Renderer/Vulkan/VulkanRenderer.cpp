//
// Created by IDKTHIS on 02.07.2025.
//

#include "VulkanRenderer.h"

#include <cassert>
#include <unordered_map>

#include "Core/CVar/CVar.h"
#include "Core/Log/Logger.h"
#include "Commands/VulkanCommandSystem.h"
#include "Devices/LogicalDevice.h"
#include "Devices/PhysicalDevice.h"
#include "Pipeline/GraphicsPipeline.h"
#include "RenderPass/VulkanRenderPass.h"
#include "Swapchain/VulkanSwapChain.h"
#include "Core/ECS/Base/UWorld.h"
#include "Core/ECS/Components/UTransformComponent.h"
#include "Core/ECS/Components/UMeshComponent.h"


#include "Core/Common/Vertex.h"
#include "Core/ECS/Components/UCameraComponent.h"
#include "Core/Utils/FileSystem.h"
#include "Platform/Renderer/Common/IVertexLayout.h"
#include "Application/Application.h"
#include "Common/VulkanTexture.h"
#include "Core/ECS/Player/APawn.h"
#include "Core/ECS/Player/APlayerController.h"
#include "Core/ECS/Player/ULocalPlayer.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_vulkan.h"
#include <SDL3/SDL_vulkan.h>

#include "Core/VulkanRenderObject.h"
#include "Core/VulkanResourceManager.h"

DECLARE_CONVAR_MINMAX("r_max_frames_in_flight", 2, 1, 4, "Maximum number of frames in flight for swapchain", CVAR_ARCHIVE);

static vk::DescriptorPool g_imguiDescriptorPool = {};
static bool g_imguiInitialized = false;

VulkanRenderer::VulkanRenderer()
{
	m_context = std::make_unique<VulkanContext>();

	m_context->instance        = std::make_unique<VulkanInstance>(m_context.get());
	m_context->physicalDevice  = std::make_unique<PhysicalDevice>(m_context.get());
	m_context->logicalDevice   = std::make_unique<LogicalDevice>(m_context.get());
	m_context->swapchain       = std::make_unique<VulkanSwapChain>(m_context.get());
	m_context->renderPass      = std::make_unique<VulkanRenderPass>(m_context.get());

	m_context->pipelines.emplace(
		PipelineType::Graphics,
		std::make_unique<GraphicsPipeline>(m_context.get())
	);
	m_context->commandSystem   = std::make_unique<VulkanCommandSystem>(m_context.get());

	if (!m_shaderPak.Open(Engine::FileSystem::GetWorkingDirectory() + "Content/Paks/VulkanShaders.voxpak"))
	{
		LOG_FATAL("Vulkan", "Failed to open VulkanShaders.voxpak");
	}

	m_cameraUBO = std::make_unique<VulkanCameraUBO>(m_context.get());
	m_renderObjectManager = std::make_unique<VulkanResourceManager>(m_context.get());

} ;
VulkanRenderer::~VulkanRenderer() {
	//Cleanup();
};
VulkanShader* vulkanShader = nullptr;;
bool VulkanRenderer::Init(IWindow *window, UWorld* world) {
	if (!m_context->instance->Init()) {
		return  false;
	}
	VkSurfaceKHR c_surface;
	SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(window->GetNativeHandle()), m_context->instance->GetInstance(), nullptr, &c_surface); //TODO: Make CreateSurface in IWindow.
	m_context->surface = c_surface;
	vk::Extent2D windowExtent{ window->GetWidth(), window->GetHeight() };
	bool isSuccess =
		   m_context->physicalDevice->Init() &&
		   m_context->logicalDevice->Init() &&
		   m_context->swapchain->Init(windowExtent) &&
		   m_context->renderPass->Init();
	if (!isSuccess) {
		LOG_FATAL("Vulkan", "Failed to initialize Vulkan components.");
		return false;
	}
	vulkanShader = new VulkanShader(m_context->logicalDevice.get());
	vulkanShader->LoadFromSource(m_shaderPak.ReadFileWithOverrideString("SimpleRectangle.shader"));

	m_window = window;
	m_context->pipelines[PipelineType::Graphics]->SetShader(vulkanShader);
	InitImGuiForVulkan(window);

    m_cameraUBO->PreInit();
	auto cameraLayout = m_cameraUBO->GetDescriptorSetLayout();

	vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	vk::DescriptorSetLayoutCreateInfo samplerLayoutInfo{};
	samplerLayoutInfo.bindingCount = 1;
	samplerLayoutInfo.pBindings = &samplerLayoutBinding;

	m_textureLayout = m_context->logicalDevice->GetHandle().createDescriptorSetLayout(samplerLayoutInfo);
	m_context->pipelines[PipelineType::Graphics]->AddDescriptorSetLayout(cameraLayout);
	m_context->pipelines[PipelineType::Graphics]->AddDescriptorSetLayout(m_textureLayout);


	m_context->pipelines[PipelineType::Graphics]->Init();
	m_context->swapchain->CreateFramebuffers(m_context->renderPass.get());
	if (!m_context->commandSystem->Init()) {
		return false;
	}

	m_cameraUBO->Init();
	m_imageAvailableSemaphores.resize(GET_CVAR(int, "r_max_frames_in_flight"));
	m_renderFinishedSemaphores.resize(m_context->swapchain->GetImageCount());
	m_inFlightFences.resize(GET_CVAR(int, "r_max_frames_in_flight"));
	for (int i = 0; i < GET_CVAR(int, "r_max_frames_in_flight"); ++i) {
		m_imageAvailableSemaphores[i] = m_context->logicalDevice->GetHandle().createSemaphore({});
		m_inFlightFences[i] = m_context->logicalDevice->GetHandle().createFence({vk::FenceCreateFlagBits::eSignaled});
	}
	for (uint32_t i = 0; i < m_context->swapchain->GetImageCount(); ++i) {
		m_renderFinishedSemaphores[i] = m_context->logicalDevice->GetHandle().createSemaphore({});
	}
	m_world = world;
	m_renderObjectManager->Initialize();
	return true;
}

void VulkanRenderer::BeginFrame() {
	m_renderedVertices = 0;
	m_context->logicalDevice->GetHandle().waitForFences(m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	m_context->swapchain->BeginRender(m_imageAvailableSemaphores[m_currentFrame]);
	m_context->logicalDevice->GetHandle().resetFences(m_inFlightFences[m_currentFrame]);
	m_context->commandSystem->GetCommandBuffer(m_currentFrame).reset();
}

void VulkanRenderer::ProcessRender() {
    auto& cmd = m_context->commandSystem->GetCommandBuffer(m_currentFrame);

    vk::CommandBufferBeginInfo beginInfo{};
    cmd.begin(beginInfo);

    UCameraComponent* camera = Engine::Application::Get()->GetLocalPlayer()->GetController()->GetPawn()->GetComponent<UCameraComponent>();
    if (camera) {
        CameraData camera_data = {
            camera->GetViewMatrix(),
            camera->GetProjectionMatrix()
        };
        m_cameraUBO->Update(&cmd, camera_data);
    }

    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.renderPass = m_context->renderPass->GetHandle();
    renderPassBeginInfo.framebuffer = m_context->swapchain->GetFramebuffer(m_context->swapchain->GetCurrentImageIndex());
    renderPassBeginInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassBeginInfo.renderArea.extent = m_context->swapchain->GetSwapExtent();
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_context->pipelines[PipelineType::Graphics]->GetPipeline());
	m_renderObjectManager->UpdateWorldState(m_world);
	m_renderObjectManager->RenderObjects(cmd, m_cameraUBO.get(), m_currentFrame);
	if (g_imguiInitialized) {

		ImGui::Begin("Debug");
		ImGui::Text("Vulkan + ImGui");
		ImGui::Text("FPS: %.1f", 1.0f/ImGui::GetIO().DeltaTime);
		ImGui::Text("Rendered vertices: %llu", m_renderedVertices);
		ImGui::End();
		{
			ImDrawList* drawList = ImGui::GetBackgroundDrawList();
			ImVec2 displaySize = ImGui::GetIO().DisplaySize;
			ImVec2 center(displaySize.x * 0.5f, displaySize.y * 0.5f);

			float size = 8.0f;
			float thickness = 2.0f;
			ImU32 color = IM_COL32(255, 255, 255, 255);

			drawList->AddLine(
				ImVec2(center.x - size, center.y),
				ImVec2(center.x + size, center.y),
				color,
				thickness
			);

			drawList->AddLine(
				ImVec2(center.x, center.y - size),
				ImVec2(center.x, center.y + size),
				color,
				thickness
			);
		}

		Engine::GetCurrentContext().GetImGui()->Render(cmd);
	}

    cmd.endRenderPass();
    cmd.end();
}


void VulkanRenderer::EndFrame() {
	std::array<vk::Semaphore, 1> waitSemaphores = { m_imageAvailableSemaphores[m_currentFrame] };
	std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	std::array<vk::Semaphore, 1> signalSemaphores = { m_renderFinishedSemaphores[m_context->swapchain->GetCurrentImageIndex()] };
	vk::SubmitInfo submitInfo{};
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());;
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.pCommandBuffers = &m_context->commandSystem->GetCommandBuffer(m_currentFrame);
	submitInfo.commandBufferCount = 1;
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	m_context->logicalDevice->GetGraphicsQueue().submit(submitInfo, m_inFlightFences[m_currentFrame]);
	m_context->swapchain->Present(m_renderFinishedSemaphores[m_context->swapchain->GetCurrentImageIndex()]);
	m_currentFrame = (m_currentFrame + 1) % GET_CVAR(int, "r_max_frames_in_flight");
}


void VulkanRenderer::Cleanup() {
	LOG_INFO("Vulkan", "Cleaning up Vulkan resources.");
	m_context->logicalDevice->GetHandle().waitIdle();
	m_renderObjectManager->Cleanup();
	m_renderObjectManager.reset();
	m_meshDataMap2.clear();
	Engine::GetCurrentContext().GetImGui()->Shutdown();
	m_context->commandSystem.reset();
	m_context->swapchain.reset();

	m_context->renderPass.reset();
	m_context->pipelines[PipelineType::Graphics].reset();
	for (auto& semaphore : m_imageAvailableSemaphores) {
		if (semaphore) {
			m_context->logicalDevice->GetHandle().destroySemaphore(semaphore);
		}
	}
	m_imageAvailableSemaphores.clear();

	for (auto& semaphore : m_renderFinishedSemaphores) {
		if (semaphore) {
			m_context->logicalDevice->GetHandle().destroySemaphore(semaphore);
		}
	}
	m_renderFinishedSemaphores.clear();

	for (auto& fence : m_inFlightFences) {
		if (fence) {
			m_context->logicalDevice->GetHandle().destroyFence(fence);
		}
	}
	m_inFlightFences.clear();
	m_context->logicalDevice.reset();
	m_context->physicalDevice.reset();
	m_context->instance->GetInstance().destroySurfaceKHR(m_context->surface);
	m_context->instance.reset();
	LOG_INFO("Vulkan", "Vulkan resources cleaned up.");
}


void VulkanRenderer::RenderFrame() {
	BeginFrame();
	ProcessRender();
	EndFrame();
}

bool VulkanRenderer::InitImGuiForVulkan(IWindow* window)
{
    std::vector<vk::DescriptorPoolSize> poolSizes = {
        { vk::DescriptorType::eSampler, 1000 },
        { vk::DescriptorType::eCombinedImageSampler, 1000 },
        { vk::DescriptorType::eSampledImage, 1000 },
        { vk::DescriptorType::eStorageImage, 1000 },
        { vk::DescriptorType::eUniformTexelBuffer, 1000 },
        { vk::DescriptorType::eStorageTexelBuffer, 1000 },
        { vk::DescriptorType::eUniformBuffer, 1000 },
        { vk::DescriptorType::eStorageBuffer, 1000 },
        { vk::DescriptorType::eUniformBufferDynamic, 1000 },
        { vk::DescriptorType::eStorageBufferDynamic, 1000 },
        { vk::DescriptorType::eInputAttachment, 1000 }
    };

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    poolInfo.maxSets = 1000 * static_cast<uint32_t>(poolSizes.size());
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    g_imguiDescriptorPool = m_context->logicalDevice->GetHandle().createDescriptorPool(poolInfo);


    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_context->instance->GetInstance();
    init_info.PhysicalDevice = m_context->physicalDevice->GetHandle();
    init_info.Device = m_context->logicalDevice->GetHandle();
    init_info.QueueFamily = m_context->physicalDevice->GetGraphicsQueueFamilyIndex();
    init_info.Queue = m_context->logicalDevice->GetGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = g_imguiDescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = 3;
    init_info.ImageCount = m_context->swapchain->GetImageCount();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
	init_info.RenderPass = m_context->renderPass->GetHandle();

	Engine::GetCurrentContext().GetImGui()->InitGraphics(&init_info);


    g_imguiInitialized = true;
    return true;
}