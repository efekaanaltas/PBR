#pragma once

#include "types.h"
#include <string>
#include <vector>
#include "shader.h"
#include "texture.h"

struct Vertex
{
    v3 position;
    v2 texCoords;
    v3 normal;
    v3 tangent;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Texture> textures;

    u32 vao, vbo, ebo;

    void SetupMesh();

    Mesh(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<Texture> textures);
    void Draw(Shader& shader);
};