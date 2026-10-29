#include "Model.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Model::~Model() {
    for (auto& mesh : meshes) {
        if (mesh.vao) glDeleteVertexArrays(1, &mesh.vao);
        if (mesh.vbo) glDeleteBuffers(1, &mesh.vbo);
        if (mesh.ebo) glDeleteBuffers(1, &mesh.ebo);
    }
    if (textureID) glDeleteTextures(1, &textureID);
}

bool Model::loadFromFile(const std::string& filepath, const std::string& texturePath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    // Process ALL meshes in the file (Chassis + Wheels)
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];
        SubMesh subMesh;
        subMesh.name = mesh->mName.C_Str();

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.Normal = mesh->HasNormals() ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : glm::vec3(0.0f, 1.0f, 0.0f);
            vertex.TexCoords = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        subMesh.indexCount = indices.size();

        glGenVertexArrays(1, &subMesh.vao);
        glGenBuffers(1, &subMesh.vbo);
        glGenBuffers(1, &subMesh.ebo);

        glBindVertexArray(subMesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, subMesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        meshes.push_back(subMesh);
    }

    if (!texturePath.empty()) {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        stbi_image_free(data);
    }

    return true;
}

void Model::drawMesh(size_t meshIndex) const {
    if (meshIndex >= meshes.size()) return;
    if (textureID != 0) glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(meshes[meshIndex].vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(meshes[meshIndex].indexCount), GL_UNSIGNED_INT, 0);
}

void Model::drawMeshByName(const std::string& name) const {
    for (size_t i = 0; i < meshes.size(); i++) {
        if (meshes[i].name.find(name) != std::string::npos) {
            drawMesh(i);
            return;
        }
    }
}

void Model::drawAllExcept(const std::vector<std::string>& excludedNames) const {
    for (size_t i = 0; i < meshes.size(); i++) {
        bool exclude = false;
        for (const auto& ex : excludedNames) {
            if (meshes[i].name.find(ex) != std::string::npos) {
                exclude = true;
                break;
            }
        }
        if (!exclude) drawMesh(i);
    }
}

void Model::drawAll() const {
    for (size_t i = 0; i < meshes.size(); i++) {
        drawMesh(i);
    }
}