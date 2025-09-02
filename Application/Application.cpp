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

#include "../Core/CVar/CVar.h"
#include "../Core/Log/Logger.h"
#include "Core/ECS/BaseClasses/UWorld.h"
#include "Core/ECS/Components/UTransformComponent.h"
#include "Core/ECS/Components/UMeshComponent.h"
#include "Platform/Window/Components/WindowInputComponent.h"

using namespace Engine;

CONVAR("w_title", "VoxCore base", "Game window title", CVAR_RUNTIME_ONLY);
CONVAR("w_size_width", 1920, "Game window width", CVAR_ARCHIVE);
CONVAR("w_size_height", 1080, "Game window height", CVAR_ARCHIVE);

Application::Application() = default;

class TestCube : public AActor {
    UCLASS(TestCube);
public:
    TestCube() {
        AddComponent(std::make_shared<UTransformComponent>());
        auto mesh = std::make_shared<UMeshComponent>();
        mesh->SetWeirdMesh();
        AddComponent(mesh);
    }
};

Application::Application(std::string title, int width, int height) {
    SET_CVAR("w_title", title);
    SET_CVAR("w_size_width", width);
    SET_CVAR("w_size_height", height);
}

Application::~Application() = default;

void Application::Run() {
    Logger::instance().add_output("*", std::cout);
    LOG_INFO("Application", "Initializing engine.");
    Init();
    LOG_INFO("Application", "Initialization successful.");
    LOG_INFO("Application", "Creating window.");
    window = std::make_unique<SDL3Window>();
    if (!window->Create(GET_CVAR(int, "w_size_width"), GET_CVAR(int, "w_size_height"), GET_CVAR(std::string, "w_title"))) {
        return;
    }
    LOG_INFO("Application", "Creating Vulkan renderer.");
    renderer = std::make_unique<VulkanRenderer>();
    m_world = std::make_shared<UWorld>();
    if (!renderer->Init(window.get(), m_world.get())) {
        LOG_FATAL("Application", "Failed to initialize Vulkan renderer.");
        return;
    }

    LOG_INFO("Application", "Starting main loop.");
    MainLoop();
    LOG_INFO("Application", "Main loop terminated. Shutting down renderer.");
    renderer->Cleanup();
    LOG_INFO("Application", "Renderer cleaned up. Executing shutdown procedures.");
    Shutdown();
    LOG_INFO("Application", "Shutdown complete.");
}

void Application::Init() {

}

// Updates application state (every frame)
void Application::Update(float dt) {
    VulkanRenderer* vkRenderer = GetVulkanRenderer();
    if (!vkRenderer) return;
    Camera* camera = vkRenderer->GetCamera();
    if (!camera) return;

    float moveSpeed = (window->GetInputComponent()->IsKeyDown(KeyCode::KEY_LEFT_SHIFT) ? 8.0f : 3.0f) * dt;
    glm::vec3 move(0.0f);
    if (window->GetInputComponent()->IsKeyDown(KeyCode::KEY_W)) move.z += moveSpeed;
    if (window->GetInputComponent()->IsKeyDown(KeyCode::KEY_S)) move.z -= moveSpeed;
    if (window->GetInputComponent()->IsKeyDown(KeyCode::KEY_A)) move.x -= moveSpeed;
    if (window->GetInputComponent()->IsKeyDown(KeyCode::KEY_D)) move.x += moveSpeed;
    if (window->GetInputComponent()->IsKeyDown(KeyCode::KEY_SPACE)) move.y += moveSpeed;
    if (window->GetInputComponent()->IsKeyDown(KeyCode::KEY_LEFT_CONTROL)) move.y -= moveSpeed;
    if (glm::length(move) > 0.0f) camera->Move(move);

    static bool mouseCaptured = false;
    static bool lastRightButton = false;
    bool rightButton = window->GetInputComponent()->GetMouseState().buttons[3];
    if (rightButton && !lastRightButton) {
        mouseCaptured = !mouseCaptured;
        window->SetRelativeMouseMode(mouseCaptured);
    }
    lastRightButton = rightButton;

    if (mouseCaptured) {
        float sensitivity = 0.12f;
        camera->Rotate(window->GetInputComponent()->GetMouseState().deltaX * sensitivity, -window->GetInputComponent()->GetMouseState().deltaY * sensitivity);
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

    glm::vec3 offset = glm::vec3(1.0f, 3.0f, 2.0f);

    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            for (int z = 0; z < 3; ++z) {
                auto cube = m_world->SpawnActor<TestCube>();
                glm::vec3 pos = glm::vec3(x, y, z) * 2.0f - offset;
                cube->GetComponent<UTransformComponent>()->SetPosition(pos);
            }
        }
    }
    while (!window->ShouldClose()) {
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        window->PollEvents();
        Update(dt);
        renderer->RenderFrame();
        window->SwapBuffers();
        // FPS
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
