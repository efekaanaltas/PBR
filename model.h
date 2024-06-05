#pragma once

#include "shader.h"
#include "mesh.h"
#include "texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Model
{
	std::vector<Mesh> meshes;
	std::string directory;

	Model(const char* path)
	{
		LoadModel(path);
	}

	void Draw(Shader& shader);
	void LoadModel(std::string path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};