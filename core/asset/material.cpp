#include "../rendering/shader.h"
#include "material.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>


namespace core
{
    void Material::Use() const
    {
        glUseProgram(m_shaderProgram);

        // Bind Texture objects
        for (const auto& [name, texData] : m_textures)
        {
            if (texData.texture)
            {
                glActiveTexture(GL_TEXTURE0 + texData.slot);
                glBindTexture(GL_TEXTURE_2D, texData.texture->getId());
                GLint location = glGetUniformLocation(m_shaderProgram, name.c_str());
                if (location != -1)
                {
                    glUniform1i(location, texData.slot);
                }
            }
        }

        // Bind raw texture IDs
        for (const auto& [name, texData] : m_rawTextures)
        {
            if (texData.textureID != 0)
            {
                glActiveTexture(GL_TEXTURE0 + texData.slot);
                glBindTexture(GL_TEXTURE_2D, texData.textureID);
                GLint location = glGetUniformLocation(m_shaderProgram, name.c_str());
                if (location != -1)
                {
                    glUniform1i(location, texData.slot);
                }
            }
        }

        // Set uniforms
        for (const auto& [name, value] : m_floats)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniform1f(loc, value);
        }

        for (const auto& [name, value] : m_ints)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniform1i(loc, value);
        }

        for (const auto& [name, value] : m_bools)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniform1i(loc, value);
        }

        for (const auto& [name, value] : m_vec3s)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniform3fv(loc, 1, glm::value_ptr(value));
        }

        for (const auto& [name, value] : m_vec4s)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniform4fv(loc, 1, glm::value_ptr(value));
        }

        for (const auto& [name, value] : m_mat4s)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
        }
    }

    json Material::Serialize() const
    {
        json j;
        j["vertexShader"] = m_vertexShaderPath;
        j["fragmentShader"] = m_fragmentShaderPath;
        
        json textures = json::object();
        for (const auto& [uniformName, texData] : m_textures) {
            if (texData.texture) {
                json texInfo;
                texInfo["path"] = texData.texture->GetPath();
                texInfo["slot"] = texData.slot;
                textures[uniformName] = texInfo;
            }
        }
        j["textures"] = textures;
        
        if (!m_floats.empty()) {
            json floats = json::object();
            for (const auto& [name, value] : m_floats) floats[name] = value;
            j["floats"] = floats;
        }
        
        if (!m_ints.empty()) {
            json ints = json::object();
            for (const auto& [name, value] : m_ints) ints[name] = value;
            j["ints"] = ints;
        }
        
        if (!m_bools.empty()) {
            json bools = json::object();
            for (const auto& [name, value] : m_bools) bools[name] = (value != 0);
            j["bools"] = bools;
        }
        
        if (!m_vec3s.empty()) {
            json vec3s = json::object();
            for (const auto& [name, value] : m_vec3s) {
                vec3s[name] = { value.x, value.y, value.z };
            }
            j["vec3s"] = vec3s;
        }
        
        if (!m_vec4s.empty()) {
            json vec4s = json::object();
            for (const auto& [name, value] : m_vec4s) {
                vec4s[name] = { value.x, value.y, value.z, value.w };
            }
            j["vec4s"] = vec4s;
        }
        
        return j;
    }

    void Material::Deserialize(const json& data)
    {
        // Check if shader paths exist AND are not empty
        if (data.contains("vertexShader") && data.contains("fragmentShader")) {
            m_vertexShaderPath = data["vertexShader"].get<std::string>();
            m_fragmentShaderPath = data["fragmentShader"].get<std::string>();
            
            // Only try to load if paths are not empty
            if (!m_vertexShaderPath.empty() && !m_fragmentShaderPath.empty()) {
                try {
                    Shader shader(m_vertexShaderPath.c_str(), m_fragmentShaderPath.c_str());
                    m_shaderProgram = shader.ID;
                    printf("[Material] Loaded shaders: %s, %s\n", m_vertexShaderPath.c_str(), m_fragmentShaderPath.c_str());
                }
                catch (const std::exception& e) {
                    printf("[Material] Failed to load shaders '%s', '%s': %s\n", 
                           m_vertexShaderPath.c_str(), m_fragmentShaderPath.c_str(), e.what());
                    m_shaderProgram = 0;
                }
            }
            else {
                printf("[Material] WARNING: Shader paths are empty, material will not render!\n");
                m_shaderProgram = 0;
            }
        }
        
        if (data.contains("textures")) {
            for (auto& [uniformName, texInfo] : data["textures"].items()) {
                try {
                    std::string path = texInfo["path"].get<std::string>();
                    int slot = texInfo["slot"].get<int>();
                    
                    if (!path.empty()) {
                        auto texture = std::make_shared<Texture>(path);
                        SetTexture(uniformName, texture, slot);
                        printf("[Material] Loaded texture '%s' for uniform '%s'\n", path.c_str(), uniformName.c_str());
                    }
                    else {
                        printf("[Material] WARNING: Empty texture path for uniform '%s'\n", uniformName.c_str());
                    }
                }
                catch (const std::exception& e) {
                    printf("[Material] Failed to load texture for uniform '%s': %s\n", uniformName.c_str(), e.what());
                }
            }
        }
        
        if (data.contains("floats")) {
            for (auto& [name, value] : data["floats"].items()) {
                SetFloat(name, value.get<float>());
            }
        }
        
        if (data.contains("ints")) {
            for (auto& [name, value] : data["ints"].items()) {
                SetInt(name, value.get<int>());
            }
        }
        
        if (data.contains("bools")) {
            for (auto& [name, value] : data["bools"].items()) {
                SetBool(name, value.get<bool>());
            }
        }
        
        if (data.contains("vec3s")) {
            for (auto& [name, value] : data["vec3s"].items()) {
                try {
                    auto vec = value.get<std::vector<float>>();
                    if (vec.size() == 3) {
                        SetVec3(name, glm::vec3(vec[0], vec[1], vec[2]));
                    }
                }
                catch (const std::exception& e) {
                    printf("[Material] Failed to deserialize vec3 '%s': %s\n", name.c_str(), e.what());
                }
            }
        }
        
        if (data.contains("vec4s")) {
            for (auto& [name, value] : data["vec4s"].items()) {
                try {
                    auto vec = value.get<std::vector<float>>();
                    if (vec.size() == 4) {
                        SetVec4(name, glm::vec4(vec[0], vec[1], vec[2], vec[3]));
                    }
                }
                catch (const std::exception& e) {
                    printf("[Material] Failed to deserialize vec4 '%s': %s\n", name.c_str(), e.what());
                }
            }
        }
    }
}