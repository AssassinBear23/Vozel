#include "../../scene.h"
#include "../ComponentFactory.h"
#include "../GameObject.h"
#include "core/assimpLoader.h"
#include "core/model.h"
#include "Renderer.h"
#include <core/ObjectSystems/component.h>
#include <cstdio>
#include <glad/glad.h>
#include <imgui.h>
#include <memory>

namespace core
{
	REGISTER_COMPONENT(Renderer);

    void Renderer::Render(GLenum drawMode)
    {
        if (!m_material) return;

        m_material->Use();

        for (auto& mesh : m_meshes) {
            mesh.Render(drawMode);
        }
    }

    void Renderer::DrawGui()
    {
        ImGui::Text("Meshes: %zu", m_meshes.size());
		ImGui::Text("Material: %s", m_material ? "Set" : "Not Set");
    }

    void Renderer::OnAttach(std::weak_ptr<GameObject> owner)
    {
        Component::OnAttach(owner);
     
        if (auto go = owner.lock()) {
            if (auto scene = go->GetScene()) {
                scene->RegisterRenderer(std::static_pointer_cast<Renderer>(shared_from_this()));
                m_scene = scene;
            }
        }
    }

    void Renderer::OnDetach()
    {
        if (auto scene = m_scene.lock()) {
            scene->UnregisterRenderer(std::static_pointer_cast<Renderer>(shared_from_this()));
        }
        Component::OnDetach();
    }

    void Renderer::Deserialize(const json& data)
    {
        if (data.contains("modelPath")) {
            m_modelPath = data["modelPath"].get<std::string>();
            
            if (!m_modelPath.empty()) {
                try {
                    Model model = AssimpLoader::loadModel(m_modelPath);
                    SetMeshes(model.GetMeshes());
                    printf("[Renderer] Loaded model: %s\n", m_modelPath.c_str());
                }
                catch (const std::exception& e) {
                    printf("[Renderer] Failed to load model '%s': %s\n", m_modelPath.c_str(), e.what());
                }
            }
        }
        
        if (data.contains("material")) {
            m_material = std::make_shared<Material>();
            m_material->Deserialize(data["material"]);
        }
    }

} // namespace core