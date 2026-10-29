#ifndef MODEL_HPP
#define MODEL_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct SubMesh {
    std::string name;
    unsigned int vao = 0, vbo = 0, ebo = 0;
    size_t indexCount = 0;
};

class Model {
public:
    std::vector<SubMesh> meshes;
    unsigned int textureID = 0;

    ~Model();
    bool loadFromFile(const std::string& filepath, const std::string& texturePath = "");
    void drawMesh(size_t meshIndex) const;
    void drawMeshByName(const std::string& name) const;
    void drawAllExcept(const std::vector<std::string>& excludedNames) const;
    void drawAll() const;
};

#endif