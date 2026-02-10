#include "../../core/ObjectSystems/componentFactory.h"
#include "../../core/ObjectSystems/gameObject.h"
#include "../../core/ObjectSystems/object.h"
#include "inspectorPanel.h"

namespace editor
{
    /// <summary>
    /// Displays a context menu for component operations (Remove, Reset).
    /// </summary>
    /// <param name="comp">The component to show the context menu for.</param>
    /// <returns>The action to be deferred.</returns>
    static ComponentAction ShowComponentContextMenu(std::shared_ptr<core::Component> comp)
    {
        ComponentAction action = ComponentAction::None;

        if (ImGui::BeginPopupContextItem())
        {
            auto compTypeName = comp->GetTypeName();
            bool isNotSelectable = (compTypeName == "Transform"); // Prevent removing Transform component

            ImGui::BeginDisabled(isNotSelectable);
            if (ImGui::MenuItem("Remove Component"))
            {
                action = ComponentAction::Remove;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndDisabled();

            if (ImGui::MenuItem("Reset"))
            {
                if (compTypeName == "Transform")
                {
                    auto transformCast = std::dynamic_pointer_cast<core::Transform>(comp);
                    transformCast->position = glm::vec3(0.0f);
                    transformCast->rotation = glm::vec3(0.0f);
                    transformCast->scale = glm::vec3(1.0f);
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                    return action;
                }
                else
                {
                    action = ComponentAction::Reset;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
        return action;
    }

    /// <summary>
    /// Displays a popup menu for adding new components to the selected GameObject.
    /// Lists all registered component types except Transform.
    /// </summary>
    /// <param name="selectedObj">The GameObject to add the component to.</param>
    static void ShowAddComponentContextMenu(std::shared_ptr<core::GameObject> selectedObj)
    {
        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            const auto& registeredTypes = core::ComponentFactory::GetRegisteredTypes();

            for (const auto& typeName : registeredTypes)
            {
                if (typeName == "Transform")
                    continue;

                if (ImGui::MenuItem(typeName.c_str()))
                {
                    auto newComponent = core::ComponentFactory::Create(typeName);
                    if (newComponent)
                        selectedObj->AddComponent(newComponent);

                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }

    void InspectorPanel::draw(EditorContext& ctx)
    {
        ImGui::Begin(name(), &isVisible);
        const auto& selectedObj = ctx.currentSelectedGameObject;
        if (!selectedObj)
        {
            ImGui::Text("No GameObject selected.");
            ImGui::End();
            return;
        }

        // Use & operator to get pointer to the internal bool value
        ImGui::Checkbox("##enabled_checkbox", &selectedObj->isEnabled);
        ImGui::SameLine();
        ImGui::SeparatorText(("%s", selectedObj->GetName().c_str()));

        // Deferred action to execute after iteration
        DeferredComponentAction deferredAction;

        // List components
        const auto& components = selectedObj->GetComponents();
        int componentIndex = -1;
        for (const auto& comp : components)
        {
            componentIndex++;
            ImGui::Spacing();
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
            ImGui::PushID(componentIndex);
            ImGui::BeginChild("component", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
            
            // Collapsible header inside the box
            if (comp->GetTypeName() != "Transform")
            {
                ImGui::Checkbox("##comp_enabled", &comp->isEnabled);
                ImGui::SameLine();
            }

            bool nodeOpen = ImGui::CollapsingHeader(comp->GetTypeName().c_str(), ImGuiTreeNodeFlags_DefaultOpen);

            if (nodeOpen)
            {
                ImGui::Indent();
                comp->DrawGui();
                ImGui::Unindent();
            }

            ImGui::EndChild();
            ImGui::PopID();
            ImGui::PopStyleVar();

            ComponentAction action = ShowComponentContextMenu(comp);

            // Store the action to perform after the loop to avoid modifying the component list while iterating
            if (deferredAction.action == ComponentAction::None && action != ComponentAction::None)
            {
                deferredAction.action = action;
                deferredAction.targetComponent = comp;
                deferredAction.componentIndex = componentIndex;
            }
        }

        // Add some spacing before the button
        ImGui::Spacing();
        ImGui::Spacing();

        // Center the button with 80% width of available space
        float availWidth = ImGui::GetContentRegionAvail().x;
        float buttonWidth = availWidth * 0.8f;
        float offset = (availWidth - buttonWidth) * 0.5f;
        if (offset > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

        if (ImGui::Button("Add Component", ImVec2(buttonWidth, ImGui::GetFrameHeight())))
            ImGui::OpenPopup("AddComponentPopup");

        ShowAddComponentContextMenu(selectedObj);

        ImGui::End();

        // Process deferred actions after ImGui iteration is complete
        ProcessDeferredComponentAction(ctx, deferredAction);
    }

    void InspectorPanel::ProcessDeferredComponentAction(EditorContext& ctx, const DeferredComponentAction& deferredAction)
    {
        if (deferredAction.action == ComponentAction::None || !deferredAction.targetComponent)
            return;

        const auto& selectedObj = ctx.currentSelectedGameObject;
        if (!selectedObj)
            return;

        switch (deferredAction.action)
        {
        case ComponentAction::Remove:
        {
            deferredAction.targetComponent->Destroy();
            break;
        }
        case ComponentAction::Reset:
        {
            auto typeName = deferredAction.targetComponent->GetTypeName();
            selectedObj->RemoveComponent(deferredAction.targetComponent);
            auto newComponent = core::ComponentFactory::Create(typeName);
            if (newComponent)
                selectedObj->AddComponent(newComponent, deferredAction.componentIndex);
            break;
        }
        }
    }
}