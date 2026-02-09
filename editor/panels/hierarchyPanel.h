#pragma once
#include "../Panel.h"
#include "../../Core/scene.h"
#include "../../Core/ObjectSystems/GameObject.h"

namespace editor
{
    /// <summary>
    /// Actions that can be performed on a GameObject, deferred until after iteration.
    /// </summary>
    enum class GameObjectAction
    {
        None,
        AddChild,
        Duplicate,
        Delete
    };

    /// <summary>
    /// Data structure to hold deferred action parameters.
    /// </summary>
    struct DeferredGameObjectAction
    {
        GameObjectAction action = GameObjectAction::None;
        core::GameObject* targetGameObject = nullptr;
    };

    /// <summary>
    /// Data structure for deferred empty space actions.
    /// </summary>
    enum class EmptySpaceAction
    {
        None,
        CreateEmpty,
        CreateLight
    };

    class HierarchyPanel : public Panel
    {
    public:
        HierarchyPanel() : Panel("Hierarchy", true) {}

        void draw(EditorContext& ctx) override;

    private:
        /// <summary>
        /// Processes deferred GameObject actions after the UI iteration is complete.
        /// </summary>
        /// <param name="ctx">The editor context.</param>
        /// <param name="deferredAction">The deferred GameObject action to execute.</param>
        void ProcessDeferredGameObjectAction(EditorContext& ctx, const DeferredGameObjectAction& deferredAction);

        /// <summary>
        /// Processes deferred empty space actions after the UI iteration is complete.
        /// </summary>
        /// <param name="ctx">The editor context.</param>
        /// <param name="emptySpaceAction">The empty space action to execute.</param>
        void ProcessDeferredEmptySpaceAction(EditorContext& ctx, EmptySpaceAction emptySpaceAction);
    };
}