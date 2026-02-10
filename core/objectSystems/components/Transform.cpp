#include "../../scene.h"
#include "../ComponentFactory.h"
#include "../GameObject.h"
#include "transform.h"
#include <imgui.h>

namespace core
{
	// Registers the Transform component with the component factory
	REGISTER_COMPONENT(Transform);

    /// <summary>
/// Helper function to calculate and display world matrix for debugging.
/// </summary>
    static void ShowTransformDebugInfo(const std::shared_ptr<core::GameObject>& go)
    {
        if (!go || !go->transform) return;

        auto scene = go->GetScene();
        if (!scene) return;

        // Calculate world matrix using scene's method
        glm::mat4 worldMatrix = scene->CalculateWorldMatrix(go);
        glm::mat4 localMatrix = go->transform->GetLocalMatrix();

        if (ImGui::TreeNode("Transform Debug Info"))
        {
            ImGui::Text("Local Matrix:");
            ImGui::Indent();
            for (int i = 0; i < 4; ++i)
            {
                ImGui::Text("[%.3f, %.3f, %.3f, %.3f]",
                            localMatrix[i][0], localMatrix[i][1], localMatrix[i][2], localMatrix[i][3]);
            }
            ImGui::Unindent();

            ImGui::Spacing();
            ImGui::Text("World Matrix:");
            ImGui::Indent();
            for (int i = 0; i < 4; ++i)
            {
                ImGui::Text("[%.3f, %.3f, %.3f, %.3f]",
                            worldMatrix[i][0], worldMatrix[i][1], worldMatrix[i][2], worldMatrix[i][3]);
            }
            ImGui::Unindent();

            ImGui::Spacing();

            // Extract world position, rotation, scale
            glm::vec3 worldPos = glm::vec3(worldMatrix[3]);
            ImGui::Text("World Position: (%.3f, %.3f, %.3f)", worldPos.x, worldPos.y, worldPos.z);

            // Show parent hierarchy
            ImGui::Spacing();
            ImGui::Text("Parent Hierarchy:");
            ImGui::Indent();
            auto currentGO = go;
            int depth = 0;
            while (auto parent = currentGO->GetParent().lock())
            {
                printf("In the loop");
                ImGui::Text("%d: %s", depth++, parent->GetName().c_str());
                if (parent->transform)
                {
                    ImGui::SameLine();
                    ImGui::TextDisabled("(%.1f, %.1f, %.1f)",
                                        parent->transform->position.x,
                                        parent->transform->position.y,
                                        parent->transform->position.z);
                }
                currentGO = parent;
            }
            if (depth == 0)
            {
                ImGui::TextDisabled("(Root object)");
            }
            ImGui::Unindent();

            ImGui::TreePop();
        }
    }

	glm::mat4 core::Transform::GetLocalMatrix() const
	{
		glm::mat4 mat(1.0f);
		mat = glm::translate(mat, position);
		mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1, 0, 0));
		mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0, 1, 0));
		mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0, 0, 1));
		mat = glm::scale(mat, scale);
		return mat;
	}

	void Transform::DrawGui()
	{
		ImGui::DragFloat3("Position", &position.x, 0.1f);
		ImGui::DragFloat3("Rotation", &rotation.x, 1.0f);
		ImGui::DragFloat3("Scale", &scale.x, 0.01f);
	
        ImGui::Separator();
        ShowTransformDebugInfo(GetOwner());
    }
} // namespace core