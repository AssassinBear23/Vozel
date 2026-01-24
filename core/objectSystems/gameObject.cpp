#include "Component.h"
#include "ComponentFactory.h"
#include "core/scene.h"
#include "editor/editor.h"
#include "GameObject.h"
#include "object.h"
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace core {
    std::shared_ptr<GameObject> GameObject::Create(std::string name)
    {
        auto go = std::shared_ptr<GameObject>(new GameObject(std::move(name)));
        go->m_scene = editor::Editor::editorCtx.currentScene;
        go->Init();
        return go;
    }

    void GameObject::SetParent(std::shared_ptr<GameObject> newParent)
    {
        // Always use shared_from_this to get a shared_ptr to self, not shared_ptr<GameObject>(this).
        auto self = std::static_pointer_cast<GameObject>(shared_from_this());

        // Check if the passed parent is the same as current, if so, do nothing.
        if (m_parent.lock() == newParent) // .lock() converts a weak_ptr to shared_ptr. This allows you to compare.
            return;

        // Remove from old parent
        if (auto old = m_parent.lock()) {
            auto& sibs = old->m_children;
            sibs.erase(std::remove(sibs.begin(), sibs.end(), self), sibs.end());
        }
        else
        {
            // Was a root, remove from scene roots
            m_scene.lock()->RemoveRootGameObject(self);
        }

        // Set new parent
        m_parent = newParent;

        // Add to new parent's children 
        if (newParent) {
            newParent->AddChild(self);
        }
        else {
            editor::Editor::editorCtx.currentScene->AddRootGameObject(self);
        }
    }

    std::weak_ptr<GameObject> GameObject::GetParent() const { return m_parent; }

    void GameObject::AddChild(const std::shared_ptr<GameObject>& child)
    {
        if (!child) return;

        m_children.push_back(child);
    }

    void GameObject::RemoveChild(const std::shared_ptr<GameObject>& child)
    {
        if (!child) return;

        // Move elements that are not equal to the _Val child to the front, and returns an iterator to the new end, then use .erase to remove the "removed" elements.
        m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end());
    }

    const std::vector<std::shared_ptr<GameObject>>& GameObject::GetChildren() const { return m_children; }

    const std::vector<std::shared_ptr<Component>>& GameObject::GetComponents() const {
        return m_components;
    }

    GameObject::GameObject(std::string name) {
        SetName(std::move(name));
    }

    void GameObject::SetChildrenEnabledState(bool enabled)
    {
        for (const auto& child : m_children) {
            child->isEnabled = enabled;
        }
    }

    void GameObject::SaveAndDisableComponents()
    {
        m_savedComponentStates.clear();
        m_savedComponentStates.reserve(m_components.size());
        
        for (const auto& component : m_components) {
            // Save the current state
            m_savedComponentStates.push_back(component->isEnabled.Get());
            // Disable the component
            component->isEnabled = false;
        }
    }

    void GameObject::RestoreComponentStates()
    {
        // Restore each component to its saved state
        for (size_t i = 0; i < m_components.size() && i < m_savedComponentStates.size(); ++i) {
            m_components[i]->isEnabled = m_savedComponentStates[i];
        }
        m_savedComponentStates.clear();
    }

    void GameObject::OnEnabledChanged(bool newValue)
    {
        // Call base implementation
        Object::OnEnabledChanged(newValue);
        
        // Propagate to children
        SetChildrenEnabledState(newValue);
        
        // Handle components with state preservation
        if (newValue) {
            RestoreComponentStates();
        } else {
            SaveAndDisableComponents();
        }
    }

    json GameObject::Serialize() const
    {
        json j;
        j["name"] = name;
        j["enabled"] = isEnabled.Get();
        
        // Serialize components
        json components = json::array();
        for (const auto& comp : m_components) {
            json compData;
            compData["type"] = comp->GetTypeName();
            compData["data"] = comp->Serialize();
            compData["enabled"] = comp->isEnabled.Get(); // Save component's individual enabled state
            components.push_back(compData);
        }
        j["components"] = components;
        
        // Serialize children recursively
        json children = json::array();
        for (const auto& child : m_children) {
            children.push_back(child->Serialize());
        }
        j["children"] = children;
        
        return j;
    }

    void GameObject::Deserialize(const json& data)
    {
        if (data.contains("name")) {
            name = data["name"].get<std::string>();
        }
        
        if (data.contains("enabled")) {
            isEnabled = data["enabled"].get<bool>();
        }
        
        // Deserialize components (skip Transform as it's auto-created)
        if (data.contains("components")) {
            for (const auto& compData : data["components"]) {
                std::string typeName = compData["type"].get<std::string>();
                
                // Transform is always created automatically, just deserialize it
                if (typeName == "Transform") {
                    if (transform && compData.contains("data")) {
                        transform->Deserialize(compData["data"]);
                    }
                    if (compData.contains("enabled")) {
                        transform->isEnabled = compData["enabled"].get<bool>();
                    }
                }
                else {
                    // Create component using factory
                    auto component = ComponentFactory::Create(typeName);
                    if (component) {
                        if (compData.contains("data")) {
                            component->Deserialize(compData["data"]);
                        }
                        AddComponent(component);
                        // Restore component's individual enabled state
                        if (compData.contains("enabled")) {
                            component->isEnabled = compData["enabled"].get<bool>();
                        }
                    }
                    else {
                        printf("[WARNING] Failed to create component type: %s\n", typeName.c_str());
                    }
                }
            }
        }
        
        // Deserialize children recursively
        if (data.contains("children")) {
            for (const auto& childData : data["children"]) {
                auto child = GameObject::Create();
                child->Deserialize(childData);
                child->SetParent(std::static_pointer_cast<GameObject>(shared_from_this()));
            }
        }
    }
} // namespace core
