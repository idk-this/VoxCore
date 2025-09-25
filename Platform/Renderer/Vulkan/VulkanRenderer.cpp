//
// Created by IDKTHIS on 02.07.2025.
//

#include "VulkanRenderer.h"

#include <cassert>
#include <SDL3/SDL_vulkan.h>
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

DECLARE_CONVAR_MINMAX("r_max_frames_in_flight", 2, 1, 4, "Maximum number of frames in flight for swapchain", CVAR_ARCHIVE);


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

	if (!m_shaderPak.Open(FileSystem::GetWorkingDirectory() + "Content/Paks/VulkanShaders.voxpak"))
	{
		LOG_FATAL("Vulkan", "Failed to open VulkanShaders.voxpak");
	}

	m_cameraUBO = std::make_unique<VulkanCameraUBO>(m_context->logicalDevice.get(), m_context->physicalDevice.get());

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


	m_context->pipelines[PipelineType::Graphics]->SetShader(vulkanShader);


    m_cameraUBO->PreInit(dynamic_cast<GraphicsPipeline*>(m_context->pipelines[PipelineType::Graphics].get()));
	auto cameraLayout = m_cameraUBO->GetDescriptorSetLayout();

	// --- Texture layout ---
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

	m_context->pipelines[PipelineType::Graphics]->SetDescriptorSetLayouts({
		cameraLayout,
		m_textureLayout
	});

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
	return true;
}

void VulkanRenderer::BeginFrame() {
	m_context->logicalDevice->GetHandle().waitForFences(m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	m_context->swapchain->BeginRender(m_imageAvailableSemaphores[m_currentFrame]);
	m_context->logicalDevice->GetHandle().resetFences(m_inFlightFences[m_currentFrame]);
	m_context->commandSystem->GetCommandBuffer(m_currentFrame).reset();
}

void VulkanRenderer::ProcessRender() {
    auto& cmd = m_context->commandSystem->GetCommandBuffer(m_currentFrame);

    vk::CommandBufferBeginInfo beginInfo{};
    cmd.begin(beginInfo);

    // Камера
    UCameraComponent* camera = Engine::Application::Get()->GetLocalPlayer()->GetController()->GetPawn()->GetComponent<UCameraComponent>();
    if (camera) {
        CameraData camera_data = {
            camera->GetViewMatrix(),
            camera->GetProjectionMatrix()
        };
        m_cameraUBO->Update(&cmd, camera_data);
    }

    // Очистка
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

    // Рендер акторов по одному
    if (m_world) {
        auto actors = m_world->GetActors();

        for (auto& actor : actors) {
            auto mesh = actor->GetComponent<UMeshComponent>();
            if (!mesh) continue;

            // Достаём или создаём данные для конкретного компонента
            auto meshDataIt = m_meshDataMap.find(mesh);
            if (meshDataIt == m_meshDataMap.end()) {
                PrepareMesh(mesh, {actor.get()});
                meshDataIt = m_meshDataMap.find(mesh);
            }

            // Обновляем instance buffer для этого актора
            if (UpdateInstanceBuffer(mesh, {actor.get()})) {
                vk::Buffer buffers[] = {
                    meshDataIt->second.vertexBuffer.GetBuffer(),
                    meshDataIt->second.instanceBuffer.GetBuffer()
                };
                vk::DeviceSize offsets[] = {0, 0};

                cmd.bindVertexBuffers(0, 2, buffers, offsets);
                cmd.bindIndexBuffer(
                    meshDataIt->second.indexBuffer.GetBuffer(),
                    0,
                    vk::IndexType::eUint32
                );

                // Дескрипторы: камера + текстура
                if (meshDataIt->second.textureSet) {
                    std::array<vk::DescriptorSet, 2> sets = {
                        m_cameraUBO->GetDescriptorSet(),
                        meshDataIt->second.textureSet
                    };
                    cmd.bindDescriptorSets(
                        vk::PipelineBindPoint::eGraphics,
                        m_context->pipelines[PipelineType::Graphics]->GetLayout(),
                        0,
                        static_cast<uint32_t>(sets.size()),
                        sets.data(),
                        0, nullptr
                    );
                } else {
                    cmd.bindDescriptorSets(
                        vk::PipelineBindPoint::eGraphics,
                        m_context->pipelines[PipelineType::Graphics]->GetLayout(),
                        0,
                        1,
                        &m_cameraUBO->GetDescriptorSet(),
                        0, nullptr
                    );
                }

                // Рисуем один экземпляр
                cmd.drawIndexed(
                    meshDataIt->second.indexCount,
                    1,
                    0, 0, 0
                );
            }
        }
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

void VulkanRenderer::PrepareMesh(UMeshComponent* mesh, const std::vector<AActor*>& actors)
{
	if (mesh->Mesh->vertices.empty()) return;
	// Vertex buffer
	std::vector<Vertex> vertices;
	vertices.reserve(mesh->Mesh->vertices.size());

	for (size_t i = 0; i < mesh->Mesh->vertices.size(); ++i) {
		glm::vec3 color = (i < mesh->Mesh->colors.size()) ? mesh->Mesh->colors[i] : glm::vec3(1.0f);
		glm::vec2 uv = (i < mesh->Mesh->texCoords.size()) ? mesh->Mesh->texCoords[i] : glm::vec2(0.0f, 0.0f);
		vertices.push_back( Vertex{ mesh->Mesh->vertices[i], color, uv } );
	}


	MeshRenderData data(m_context.get());
	data.indexCount = static_cast<uint32_t>(mesh->Mesh->indices.size());
	data.instanceCount = static_cast<uint32_t>(actors.size());

	data.vertexBuffer.Create(sizeof(Vertex) * vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer);
	data.vertexBuffer.UpdateBufferDataArray(vertices);

	data.indexBuffer.Create(sizeof(uint32_t) * mesh->Mesh->indices.size(), vk::BufferUsageFlagBits::eIndexBuffer);
	data.indexBuffer.UpdateBufferDataArray(mesh->Mesh->indices);

	std::vector<InstanceData> instances;
	for (auto* actor : actors) {
		InstanceData inst{};
		auto transform = actor->GetComponent<UTransformComponent>();
		inst.model = transform ? transform->GetTransformMatrix() : glm::mat4(1.0f);
		inst.color = glm::vec3(1.0f);
		instances.push_back(inst);
	}

	data.instanceBuffer.Create(sizeof(InstanceData) * instances.size(), vk::BufferUsageFlagBits::eVertexBuffer);
	data.instanceBuffer.UpdateBufferDataArray(instances);
	if (mesh->Texture->GetData())
	{
		data.texture = std::make_unique<VulkanTexture>(m_context.get());
		data.texture->UploadFromCPU(mesh->Texture.get());
		vk::DescriptorPoolSize poolSize(vk::DescriptorType::eCombinedImageSampler, 1);
		vk::DescriptorPoolCreateInfo poolInfo({}, 1, 1, &poolSize);
		data.texturePool = m_context->logicalDevice->GetHandle().createDescriptorPool(poolInfo);

		// выделяем descriptor set
		vk::DescriptorSetAllocateInfo allocInfo(data.texturePool, 1, &m_textureLayout);
		data.textureSet = m_context->logicalDevice->GetHandle().allocateDescriptorSets(allocInfo).front();

		// обновляем descriptor
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageInfo.imageView   = data.texture->GetImageView();
		imageInfo.sampler     = data.texture->GetSampler();

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite.dstSet = data.textureSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		m_context->logicalDevice->GetHandle().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

	}else {
		data.texture = nullptr;
		data.textureSet = nullptr;
	}

	m_meshDataMap.emplace(mesh, std::move(data));
}
bool VulkanRenderer::UpdateInstanceBuffer(UMeshComponent* mesh, const std::vector<AActor*>& actors) {
	auto it = m_meshDataMap.find(mesh);
	if (it == m_meshDataMap.end()) return false;

	bool needsUpdate = false;
	std::vector<InstanceData> instances;

	for (auto* actor : actors) {
		InstanceData inst{};
		auto transform = actor->GetComponent<UTransformComponent>();
		inst.model = transform ? transform->GetTransformMatrix() : glm::mat4(1.0f);
		inst.color = glm::vec3(1.0f);
		instances.push_back(inst);

		if (!needsUpdate && transform) {
			needsUpdate = true;
		}
	}
	if (needsUpdate || it->second.instanceCount != actors.size()) {
		it->second.instanceBuffer.UpdateBufferDataArray(instances);
		it->second.instanceCount = static_cast<uint32_t>(instances.size());
		return true;
	}

	return false;
}
void VulkanRenderer::Cleanup() {
	LOG_INFO("Vulkan", "Cleaning up Vulkan resources.");
	m_context->logicalDevice->GetHandle().waitIdle();

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