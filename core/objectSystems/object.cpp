#include "Object.h"

namespace core
{
    Object::Object()
    {
        // Set up the callback for isEnabled changes
        isEnabled.SetOnChange([this](bool newValue) {
            OnEnabledChanged(newValue);
            });
    }

    void Object::SetName(std::string n) { name = std::move(n); }
    const std::string& Object::GetName() const { return name; }

    void Object::OnEnabledChanged(bool newValue)
    {
        if (m_destroyed) return;

        if (newValue)
            OnEnable();
        else
            OnDisable();
    }

    void Object::Enable()
    {
        isEnabled = true;
    }

    void Object::Disable()
    {
        isEnabled = false;
    }

    void Object::Destroy()
    {
        if (m_destroyed) return;
        m_destroyed = true; 
        OnDestroy();
    }

    bool Object::IsDestroyed() const { return m_destroyed; }
} // namespace core