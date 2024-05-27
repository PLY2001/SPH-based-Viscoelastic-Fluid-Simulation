#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include <string>

class Renderer
{

public:

	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
	void ClearColor(float ColorR, float ColorG, float ColorB, float ColorA) const;
	void ClearDepth() const;
	void CullFace(int mode);
};