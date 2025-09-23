#pragma once
#include <memory>
#include <string>
#include "Platform/Renderer/Vulkan/VulkanRenderer.h"
#include "Core/Export.h"

class ULocalPlayer;
class Logger;
class CVarManager;
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
        static Application* Get();
        // Starts the main application loop
        virtual void Run();

        VulkanRenderer* GetVulkanRenderer() {
            return dynamic_cast<VulkanRenderer*>(renderer.get());
        }
        Logger& GetLogSystem() const { return *m_logSystem; }
        CVarManager& GetCVar() const { return *m_cvar; }
        std::shared_ptr<ULocalPlayer> GetLocalPlayer() const { return m_localPlayer; }
        IWindow* GetWindow() const { return window.get(); }
        UWorld* GetWorld() const { return m_world.get(); }
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


    protected:
        static Application* m_instance;
        std::unique_ptr<Logger> m_logSystem;
        std::unique_ptr<CVarManager> m_cvar;
        std::shared_ptr<ULocalPlayer> m_localPlayer;
    };

    inline Application& GetCurrentContext() {
        return *Application::Get();
    }
}
