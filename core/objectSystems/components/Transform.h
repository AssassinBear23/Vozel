#pragma once

#include "../component.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace core
{
    class Transform : public Component
    {
    public:
        std::string GetTypeName() const override { return "Transform"; }

        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f); // Euler Angles
        glm::vec3 scale = glm::vec3(1.0f);

        Transform() = default;

        glm::mat4 GetLocalMatrix() const;

        void DrawGui() override;

        glm::vec3 forward() const
        {
            glm::quat rot = glm::quat(glm::radians(rotation));
            return glm::normalize(rot * glm::vec3(0, 0, -1));
        }

        /// <summary>
        /// Serializes the Transform component to JSON.
        /// </summary>
        json Serialize() const override
        {
            json j;
            j["position"] = { position.x, position.y, position.z };
            j["rotation"] = { rotation.x, rotation.y, rotation.z };
            j["scale"] = { scale.x, scale.y, scale.z };
            return j;
        }

        /// <summary>
        /// Deserializes the Transform component from JSON.
        /// </summary>
        void Deserialize(const json& data) override
        {
            if (data.contains("position"))
            {
                auto pos = data["position"];
                position = glm::vec3(pos[0], pos[1], pos[2]);
            }
            if (data.contains("rotation"))
            {
                auto rot = data["rotation"];
                rotation = glm::vec3(rot[0], rot[1], rot[2]);
            }
            if (data.contains("scale"))
            {
                auto scl = data["scale"];
                scale = glm::vec3(scl[0], scl[1], scl[2]);
            }
        }

    };
} // namespace core