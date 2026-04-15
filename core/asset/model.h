#pragma once

#include "../rendering/mesh.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <string>
#include <vector>

namespace core {
    class Model {
    private:
        std::vector<core::Mesh> meshes;
        glm::mat4 modelMatrix;
        std::string m_modelPath;
    public:
        Model(std::vector<core::Mesh> meshes) : meshes(meshes), modelMatrix(1) {}
        Model(std::vector<core::Mesh> meshes, const std::string& path) 
            : meshes(meshes), modelMatrix(1), m_modelPath(path) {}

        void Render(GLenum drawMode);

        void Translate(glm::vec3 translation);
        void Rotate(glm::vec3 axis, float radians);
        void Scale(glm::vec3 Scale);
        glm::mat4 GetModelMatrix() const;
        
        const std::vector<core::Mesh>& GetMeshes() const { return meshes; }
        const std::string& GetModelPath() const { return m_modelPath; }
    };
}