#ifndef CUBE_MESH_HPP
#define CUBE_MESH_HPP

#include <glad/glad.h>

class CubeMesh {
public:
    unsigned int vao, vbo;

    CubeMesh();
    ~CubeMesh();

    void draw() const;
};

#endif