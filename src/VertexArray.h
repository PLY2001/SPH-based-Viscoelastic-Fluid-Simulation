#pragma once
#include "VertexBuffer.h"
#include "VertexAttribLayout.h"



class VertexArray
{
private:
	unsigned int& RendererID;
public:
	VertexArray(unsigned int& RendererID);
	~VertexArray();

	void AddBuffer(const unsigned int& vbID, const VertexAttribLayout& layout);

	void Bind() const;
	void Unbind() const;
};