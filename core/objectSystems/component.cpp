#include "Component.h"
#include "GameObject.h" // only needed for the full type; safe include here
#include <imgui.h>

namespace core 
{
    void Component::OnAttach(std::weak_ptr<GameObject> owner) {
        m_owner = std::move(owner);
    }

    std::shared_ptr<GameObject> Component::GetOwner() const {
        return m_owner.lock();
    }

    void Component::DrawGui()
    {
		ImGui::Text("No GUI implemented for %s", GetTypeName().c_str());
    }

    void Component::OnDestroy()
    {
        if (auto owner = m_owner.lock())
            owner->RemoveComponent(std::dynamic_pointer_cast<Component>(shared_from_this()));
    }
} // namespace core
