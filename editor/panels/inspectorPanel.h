#pragma once

#include "../panel.h"
#include <memory>

namespace core
{
    class Component;
}

namespace editor
{
    /// <summary>
    /// Actions that can be performed on a component, deferred until after iteration.
    /// </summary>
    enum class ComponentAction
    {
        None,
        Remove,
        Reset
    };

    /// <summary>
    /// Data structure to hold deferred component action parameters.
    /// </summary>
    struct DeferredComponentAction
    {
        ComponentAction action = ComponentAction::None;
        std::shared_ptr<core::Component> targetComponent = nullptr;
        int componentIndex = -1;
    };

    class InspectorPanel : public Panel
    {
    public:
        InspectorPanel() : Panel("Inspector", true) {}
        void draw(EditorContext& ctx) override;

    private:
        /// <summary>
        /// Processes deferred component actions after the UI iteration is complete.
        /// </summary>
        /// <param name="ctx">The editor context.</param>
        /// <param name="deferredAction">The deferred component action to execute.</param>
        void ProcessDeferredComponentAction(EditorContext& ctx, const DeferredComponentAction& deferredAction);
    };
} // namespace editor