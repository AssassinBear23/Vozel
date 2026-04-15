#include "core/camera.h"
#include "core/rendering/frameBuffer.h"
#include "game.h"
#include "inputManager.h"
#include <algorithm>
#include <core/rendering/postProcessing/postProcessingManager.h>
#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace game
{
    Game::~Game()
    {
        if (m_initialized)
            shutdown();
    }

    static std::vector<float> fpsHistory;
    static float GetAverageFPS(float deltaTime)
    {
        fpsHistory.push_back(1.0f / deltaTime);
        if (fpsHistory.size() > 60)
            fpsHistory.erase(fpsHistory.begin());
        float sum = 0.0f;
        for (float fps : fpsHistory)
            sum += fps;
        return sum / fpsHistory.size();
    }

    static void APIENTRY glDebugOutput(GLenum source,
        GLenum type,
        unsigned int id,
        GLenum severity,
        GLsizei length,
        const char* message,
        const void* userParam)
    {
        // ignore non-significant error/warning codes
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        std::cout << "---------------" << std::endl;
        std::cout << "Debug message (" << id << "): " << message << std::endl;

        switch (source)
        {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
        } std::cout << std::endl;

        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
        } std::cout << std::endl;

        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
        } std::cout << std::endl;
        std::cout << std::endl;

        // Break on errors and warnings in debug builds
        if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
        {
            //__debugbreak();  // MSVC debugger break - inspect call stack here!
        }
    }

    static std::string SanitizeFilename(const std::string& name)
    {
        std::string safe = name;
        const std::string illegal = R"(<>:"/\|?*)";
        for (char c : illegal)
        {
            std::replace(safe.begin(), safe.end(), c, '_');
        }
        return safe;
    }

    bool Game::init(const char* glsl_version)
    {
        // Initialize GLFW
        if (!glfwInit())
        {
            printf("[GAME] Failed to initialize GLFW\n");
            return false;
        }

        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

        m_window = glfwCreateWindow(800, 600, "FinalEngine Editor", nullptr, nullptr);
        if (m_window == nullptr)
        {
            printf("[EDITOR] Failed to create GLFW window\n");
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(m_window);

        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            printf("[EDITOR] Failed to initialize GLAD\n");
            return false;
        }

        // Setup OpenGL debug callback
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }

        // Setup OpenGL state
        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::vec4 clearColor = glm::vec4(0);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

        // Initialize post-processing manager
        m_postProcessingManager = std::make_shared<core::postProcessing::PostProcessingManager>();
        m_postProcessingManager->Initialize();

        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(0, 0);
        style.FramePadding = ImVec2(4, 4);

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Initialize input manager
        m_inputManager = std::make_unique<InputManager>();
        m_inputManager->Initialize(m_window);

        // Initialize editor camera
        m_playerCamera = std::make_unique<core::Camera>(
            glm::vec3(0.0f, 0.0f, 10.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // Create scene render framebuffer
        m_viewRenderBuffer = std::make_unique<core::FrameBuffer>(
            "SceneFBO",
            core::FrameBufferSpecifications{
                800, 600,
                core::AttachmentType::COLOR_DEPTH,
                2
            }
        );

        m_initialized = true;
        printf("[GAME] Successfully initialized\n");
        return true;
    }

    void Game::shutdown()
    {
        if (!m_initialized) return;

        // Cleanup ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // Cleanup GLFW
        if (m_window)
        {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        glfwTerminate();

        m_initialized = false;
        printf("[GAME] Shutdown complete\n");
    }

    void Game::run()
    {
        if (!m_initialized)
        {
            printf("[GAME] Cannot run - not initialized!\n");
            return;
        }

        m_isRunning = true;

        double currentTime = glfwGetTime();
        m_deltaTime = 0.0f;

        // Main editor loop
        while (m_isRunning && !glfwWindowShouldClose(m_window))
        {
            glfwPollEvents();

            beginFrame();
            draw();

            // Render the 3D scene
            renderScene(m_deltaTime);

            endFrame();
            glfwSwapBuffers(m_window);

            // Calculate delta time
            double finishFrameTime = glfwGetTime();
            m_deltaTime = static_cast<float>(finishFrameTime - currentTime);
            currentTime = finishFrameTime;
        }

        printf("[EDITOR] Main loop ended\n");
    }

    void Game::renderScene(float deltaTime)
    {
        int vw = getViewportWidth();
        int vh = getViewportHeight();

        if (!m_viewRenderBuffer)
            return;

        m_viewRenderBuffer->Resize(vw, vh);

        core::FrameBuffer* viewportFrameBuffer = GetFrameBuffer();

        if (vw > 0 && vh > 0)
        {
            m_viewRenderBuffer->BindAndClear(vw, vh);

            // Process input if viewport is focused
            if (viewportFocused() && m_inputManager && m_playerCamera)
                m_inputManager->ProcessInput(m_window, m_playerCamera.get(), deltaTime);

            // Render 3D scene
            if (m_playerCamera)
            {
                glm::mat4 view = m_playerCamera->GetViewMatrix();
                glm::mat4 projection = m_playerCamera->GetProjectionMatrix(vw, vh);
                // TODO: render the current view
            }

            // Apply post-processing if effects are registered
            if (m_postProcessingManager && viewportFrameBuffer)
            {
                m_postProcessingManager->ProcessStack(
                    *m_viewRenderBuffer,
                    *viewportFrameBuffer,
                    vw, vh
                );
            }

            m_viewRenderBuffer->Unbind();
        }
    }

    void Game::beginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Game::draw()
    {
        // TODO: Add draw functionality
    }

    void Game::endFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    core::FrameBuffer* Game::GetFrameBuffer() const
    {
        return m_viewRenderBuffer.get();
    }

    int Game::getViewportWidth() const
    {
        return 800;  // TODO: Get from viewport panel
    }

    int Game::getViewportHeight() const
    {
        return 600;  // TODO: Get from viewport panel
    }

    bool Game::viewportFocused() const
    {
        return true;  // TODO: Check if viewport panel is focused
    }
}