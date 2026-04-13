#pragma once
#include <core/camera.h>
#include <core/rendering/frameBuffer.h>
#include <core/rendering/postProcessing/postProcessingManager.h>
#include <core/rendering/shader.h>
#include <game/inputManager.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <memory>
#include <vector>

namespace core { class Camera; }

namespace game
{
    class Game {
    public:
        Game() = default;
        ~Game();

        bool init(const char* glsl_version = "#version 430");
        void shutdown();

        void run();  // Main loop now lives here

        void beginFrame();   // ImGui new frame
        void draw();         // Dockspace + main menu + panels
        void endFrame();     // Render draw data

        // Viewport render target (owned by ViewportPanel; Editor proxies these)
        core::FrameBuffer* GetFrameBuffer() const;
        int    getViewportWidth() const;
        int    getViewportHeight() const;
        bool   viewportFocused() const;

        GLFWwindow* GetWindow() const { return m_window; }
    private:
        void renderScene(float deltaTime);

        bool m_isRunning = false;

        GLFWwindow* m_window = nullptr;
        bool m_initialized = false;

        float m_deltaTime = 0.0f;

        // Editor-owned resources
        std::shared_ptr<core::postProcessing::PostProcessingManager> m_postProcessingManager;
        std::unique_ptr<InputManager> m_inputManager;
        std::unique_ptr<core::Camera> m_playerCamera;
        std::unique_ptr<core::FrameBuffer> m_viewRenderBuffer;
        GLuint m_uboLights = 0;

        // Shaders for default scenes
        std::unique_ptr<core::Shader> m_modelShader;
        std::unique_ptr<core::Shader> m_textureShader;
        std::unique_ptr<core::Shader> m_lightBulbShader;
        std::unique_ptr<core::Shader> m_litSurfaceShader;
    };
}