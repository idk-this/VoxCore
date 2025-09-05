#pragma once
#include <memory>
#include <string>
#include "../Platform/Renderer/Vulkan/VulkanRenderer.h"
#include "Core/Export.h"

class IWindow;
class IRenderer;

namespace Engine {

    /**
     * Base class for the engine application.
     * Defines the lifecycle: initialization, main loop, shutdown.
     */
    class VOXCORE_API Application {
    public:
        Application();
        virtual ~Application();

        // Starts the main application loop
        virtual void Run();

        VulkanRenderer* GetVulkanRenderer() {
            return dynamic_cast<VulkanRenderer*>(renderer.get());
        }

    protected:
        // Initializes application resources
        virtual void Init();
        // Updates application state (called every frame)
        virtual void Update(float dt);
        // Shuts down the application and releases resources
        virtual void Shutdown();

        std::unique_ptr<IWindow> window;
        std::unique_ptr<IRenderer> renderer;

    protected:
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void MainLoop();
        bool isRunning = true;
        std::shared_ptr<UWorld> m_world;
        std::string appName = "Engine Application";
        std::string appVersion = "1.0.0";
    };
}
