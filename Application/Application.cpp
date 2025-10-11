//
// Created by IDKTHIS on 02.07.2025.
//

// Engine/Core/Application/Application.cpp

#include "Application.h"
#include "../Platform/Window/SDL3/SDL3Window.h"
#include "../Platform/Renderer/Vulkan/VulkanRenderer.h"
#include <chrono>
#include <utility>
#include <SDL3/SDL.h>
#include <sstream>

#include "imgui.h"
#include "../Core/CVar/CVar.h"
#include "../Core/Log/Logger.h"
#include "Core/ECS/Base/UWorld.h"
#include "Core/ECS/Components/UCameraComponent.h"
#include "Core/ECS/Components/UTransformComponent.h"
#include "Core/ECS/Components/UMeshComponent.h"
#include "Core/UI/Designer/DataBinding.h"
#include "Core/UI/Designer/XMLParser.h"
#include "Core/Utils/FileSystem.h"
#include "Platform/Window/Components/WindowInputComponent.h"

using namespace Engine;
Engine::Application* Engine::Application::m_instance = nullptr;

class MyDataContext : public UISystem::SimpleDataContext {
public:
    MyDataContext() {
        SetProperty("title", "My Application");
        SetProperty("counter", "0");

        BindEvent("IncrementCounter", [this]() {
            int count = std::stoi(GetProperty("counter"));
            SetProperty("counter", std::to_string(count + 1));
        });

        BindEvent("PrintHello", []() {
            std::cout << "Hello from button!" << std::endl;
        });
    }
};
Application::Application()
{

    m_cvar = std::make_unique<CVarManager>(CVarRegistry::Instance().GetDeclarations());
    m_logSystem = std::make_unique<Logger>();
    m_instance = this;
    m_imgui = std::make_shared<ImGuiWrapper>(ImGuiBackendGraphicsAPI::Vulkan, ImGuiBackendWindowAPI::SDL3);
};

class LocalPlayer : public AActor {
    UCLASS(localPlayer);
public:
    LocalPlayer() {
        AddComponent(std::make_shared<UTransformComponent>());
        AddComponent(std::make_shared<UCameraComponent>());
    }
};

Application::~Application() = default;

Application* Application::Get()
{
    return m_instance;
}

void Application::Run() {

}
std::shared_ptr<LocalPlayer> testPlayer;
void Application::Init() {
    //CVarManager::Instance().LoadFromFile(FileSystem::GetWorkingDirectory() + "/Config/CVars.cfg");
}

// Updates application state (every frame)
void Application::Update(float dt) {
    m_world->Update(dt);
    if (!testPlayer)
    {
        testPlayer = m_world->SpawnActor<LocalPlayer>();
    }



}

// Shuts down the application and releases resources
void Application::Shutdown() {
    // To be implemented: here will be shutdown of modules, resources, convars, etc.
}

void Application::MainLoop() {
    auto lastTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    float fpsTimer = 0.0f;
    auto dataContext = std::make_shared<MyDataContext>();

    // Парсинг UI из XML
    UISystem::XMLUIParser parser;
    auto uiRoot = parser.ParseUIFile("text_ui.xml", dataContext.get());
    while (!window->ShouldClose()) {
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        window->PollEvents();
        Engine::GetCurrentContext().GetImGui()->NewFrameGraphics();
        Engine::GetCurrentContext().GetImGui()->NewFrameWindow();
        ImGui::NewFrame();
        if (uiRoot) {
            uiRoot->Render();
        }
        Update(dt);
        renderer->RenderFrame();
        window->SwapBuffers();
        frameCount++;
        fpsTimer += dt;
        if (fpsTimer >= 1.0f) {
            std::ostringstream oss;
            oss << "VoxCraft Beta | FPS: " << frameCount;
            window->SetTitle(oss.str());
            frameCount = 0;
            fpsTimer = 0.0f;
        }
    }

}
