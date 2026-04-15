#pragma once

#include <glad/glad.h>
#include <string>

namespace core {

    class Texture {
    private:
        GLuint id;
        std::string m_path;

    public:
        Texture(const std::string& path);

        GLuint getId() const { return id; }
        const std::string& GetPath() const { return m_path; }
    };

}
