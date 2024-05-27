#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#define VERTEX
struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	//glm::vec3 Tangent;
	glm::vec2 Texcoord;
};

struct SimpleVertex
{
	glm::vec3 Position;
	glm::vec2 Texcoord;
};

struct FullVertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec2 Texcoord;
};


class VertexBuffer
{
private:
	unsigned int RendererID;
public:
	VertexBuffer(unsigned int& RendererID,const Vertex* data, unsigned int size);
	VertexBuffer(unsigned int& RendererID, const float* data, unsigned int size);
	VertexBuffer(unsigned int& RendererID, const glm::vec3* data, unsigned int size);
	VertexBuffer(unsigned int& RendererID, const SimpleVertex* data, unsigned int size);
	~VertexBuffer();
	void Bind() const;
	void Unbind() const;

};