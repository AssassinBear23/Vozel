#include "hierarchyPanel.h"
#include "../../Core/sceneManager.h"
#include "../../core/scene.h"
#include "../../core/objectSystems/gameObject.h"
#include "../../core/objectSystems/componentFactory.h"

namespace editor
{
    static DeferredGameObjectAction ShowGameObjectContextMenu(core::GameObject* go, EditorContext& ctx)
    {
        DeferredGameObjectAction deferredAction;

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Add Child"))
            {
                deferredAction.action = GameObjectAction::AddChild;
                deferredAction.targetGameObject = go;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Duplicate"))
            {
                deferredAction.action = GameObjectAction::Duplicate;
                deferredAction.targetGameObject = go;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Delete", "Del"))
            {
                deferredAction.action = GameObjectAction::Delete;
                deferredAction.targetGameObject = go;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        return deferredAction;
    }

    static EmptySpaceAction ShowEmptySpaceContextMenu(EditorContext& ctx)
    {
        EmptySpaceAction action = EmptySpaceAction::None;

        // Open popup when right-clicking on empty space in the window
        if (ImGui::BeginPopupContextWindow("HierarchyEmptySpaceMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            auto currentScene = ctx.sceneManager->GetCurrentScene();

            if (currentScene)
            {
                if (ImGui::MenuItem("Create Empty GameObject"))
                {
                    action = EmptySpaceAction::CreateEmpty;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::Separator();

                // Optional: Add Categorized Submenus
                if (ImGui::BeginMenu("Lighting"))
                {
                    if (ImGui::MenuItem("Light"))
                    {
                        action = EmptySpaceAction::CreateLight;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndMenu();
                }
            }
            else
            {
                ImGui::TextDisabled("No scene loaded");
            }

            ImGui::EndPopup();
        }

        return action;
    }

    static void DrawGameObjectNode(const std::shared_ptr<core::GameObject>& go, EditorContext& ctx, DeferredGameObjectAction& outAction)
    {
        if (!go) return;

        ImGui::PushID(go.get());

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

        // Highlight if selected
        if (ctx.currentSelectedGameObject == go)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        // Leaf node if no children
        if (go->GetChildren().empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        bool nodeOpen = ImGui::TreeNodeEx(go->GetName().c_str(), flags);

        // Handle selection
        if (ImGui::IsItemClicked())
        {
            ctx.currentSelectedGameObject = go;
        }

        DeferredGameObjectAction contextAction = ShowGameObjectContextMenu(go.get(), ctx);
        
        // Propagate the first non-None action up
        if (outAction.action == GameObjectAction::None && contextAction.action != GameObjectAction::None)
        {
            outAction = contextAction;
        }

        // Draw children if node is open
        if (nodeOpen && !go->GetChildren().empty())
        {
            for (const auto& child : go->GetChildren())
            {
                DrawGameObjectNode(child, ctx, outAction);
            }
            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    void HierarchyPanel::draw(EditorContext& ctx)
    {
        ImGui::Begin("Hierarchy", &isVisible);

        auto currentScene = ctx.sceneManager->GetCurrentScene();

        // Check for empty space context menu actions
        EmptySpaceAction emptySpaceAction = ShowEmptySpaceContextMenu(ctx);

        // Deferred action for GameObject operations
        DeferredGameObjectAction deferredAction;

        if (currentScene)
        {
            if (ImGui::TreeNodeEx(currentScene->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (const auto& root : currentScene->Roots())
                {
                    DrawGameObjectNode(root, ctx, deferredAction);
                }
                ImGui::TreePop();
            }
        }
        else
        {
            ImGui::TextDisabled("No scene loaded");
        }

        ImGui::End();

        // Process deferred actions after ImGui iteration is complete
        ProcessDeferredGameObjectAction(ctx, deferredAction);
        ProcessDeferredEmptySpaceAction(ctx, emptySpaceAction);
    }

    void HierarchyPanel::ProcessDeferredGameObjectAction(EditorContext& ctx, const DeferredGameObjectAction& deferredAction)
    {
        if (deferredAction.action == GameObjectAction::None || !deferredAction.targetGameObject)
            return;

        switch (deferredAction.action)
        {
        case GameObjectAction::AddChild:
        {
            auto newChild = core::GameObject::Create("New Child");
            newChild->SetParent(std::static_pointer_cast<core::GameObject>(deferredAction.targetGameObject->shared_from_this()));
            break;
        }
        case GameObjectAction::Duplicate:
        {
            auto newDuplicate = core::GameObject::Create(deferredAction.targetGameObject->GetName() + " Copy");
            if (auto parent = deferredAction.targetGameObject->GetParent().lock())
                newDuplicate->SetParent(parent);
            else
                newDuplicate->SetParent(nullptr);

            // TODO: Deep copy components and children.
            break;
        }
        case GameObjectAction::Delete:
        {
            deferredAction.targetGameObject->Destroy();
            if (ctx.currentSelectedGameObject.get() == deferredAction.targetGameObject)
            {
                ctx.currentSelectedGameObject = nullptr;
            }
            break;
        }
        }
    }

    void HierarchyPanel::ProcessDeferredEmptySpaceAction(EditorContext& ctx, EmptySpaceAction emptySpaceAction)
    {
        if (emptySpaceAction == EmptySpaceAction::None)
            return;

        auto currentScene = ctx.sceneManager->GetCurrentScene();
        if (!currentScene)
            return;

        switch (emptySpaceAction)
        {
        case EmptySpaceAction::CreateEmpty:
        {
            auto newObject = core::GameObject::Create("New GameObject");
            currentScene->AddRootGameObject(newObject);
            ctx.currentSelectedGameObject = newObject;
            break;
        }
        case EmptySpaceAction::CreateLight:
        {
            auto newObject = core::GameObject::Create("Light");

            auto light = core::ComponentFactory::Create("Light");
            if (light) newObject->AddComponent(light); // When gizmos are implemented, the light gizmo can show the lights position (currently it will be invisible).

            currentScene->AddRootGameObject(newObject);
            ctx.currentSelectedGameObject = newObject;
            break;
        }
        }
    }

}