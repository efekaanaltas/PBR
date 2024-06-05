#include "model.h"

std::vector<Texture> loadedTextures;

void Model::Draw(Shader& shader)
{
	for (u32 i = 0; i < meshes.size(); i++)
	{
		meshes[i].Draw(shader);
	}
}

void Model::LoadModel(std::string path)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		printf("Error loading model: %s\n", import.GetErrorString());
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));
	ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (u32 i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene));
	}

	for (u32 i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<u32> indices;
	std::vector<Texture> textures;

	for (u32 i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		v3 pos;
		pos.x = mesh->mVertices[i].x;
		pos.y = mesh->mVertices[i].y;
		pos.z = mesh->mVertices[i].z;
		vertex.position = pos;

		v3 normal;
		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;
		vertex.normal = normal;

		if (mesh->mTextureCoords[0])
		{
			v2 uv;
			uv.x = mesh->mTextureCoords[0][i].x;
			uv.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoords = uv;

			v3 tangent;
			tangent.x = mesh->mTangents[i].x;
			tangent.y = mesh->mTangents[i].y;
			tangent.z = mesh->mTangents[i].z;
			vertex.tangent = tangent;
		}
		else
		{
			vertex.texCoords = v2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}

	for (u32 i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (u32 j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "roughness");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		std::vector<Texture> aoMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, "ao");
		textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());

		std::vector<Texture> metallicMaps = LoadMaterialTextures(material, aiTextureType_METALNESS, "metallic");
		textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());

	}

	return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;
	for (u32 i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		b32 skip = false;
		for (u32 j = 0; j < loadedTextures.size(); j++)
		{
			if (std::strcmp(loadedTextures[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(loadedTextures[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			Texture texture;
			char fullPath[512];
			sprintf_s(fullPath, "%s/%s", directory.c_str(), str.C_Str());

			bool gammaCorrect = false;
			if (typeName == "diffuse")
			{
				gammaCorrect = true;
			}

			printf("Loading texture from %s\n", fullPath);
			texture.id = LoadTexture(fullPath, gammaCorrect);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			loadedTextures.push_back(texture);
		}
	}
	return textures;
}
