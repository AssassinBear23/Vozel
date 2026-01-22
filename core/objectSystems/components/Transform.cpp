#include "../ComponentFactory.h"
#include "transform.h"
#include <imgui.h>

namespace core
{
	// Registers the Transform component with the component factory
	REGISTER_COMPONENT(Transform);

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
	}
} // namespace core