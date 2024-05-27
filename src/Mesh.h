#pragma once
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <vector>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"

#include "ErrorCheck.h"

#ifndef VERTEX
struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	//glm::vec3 Tangent;
	glm::vec2 Texcoord;
};
#endif

struct myTexture
{
	unsigned int slot;
	std::string type;
	aiString path;
};


class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<myTexture> textures;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<myTexture> textures);
	void Draw(Shader& shader);
	void DrawInstanced(Shader& shader, int amount);
	unsigned int vaID, vbID, ibID;
private:
	void SetMesh();

	
	
};