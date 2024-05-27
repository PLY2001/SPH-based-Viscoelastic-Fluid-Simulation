#pragma once
#include <vector>
#include <GL/glew.h>
struct VertexAttribElement
{
	unsigned int type;
	unsigned int count;
	unsigned char normalized;

	static unsigned int GetSizeOfType(unsigned int type)
	{
		switch (type)
		{
		case GL_FLOAT:return 4;
		case GL_UNSIGNED_INT: return 4;
		case GL_UNSIGNED_BYTE:return 1;
		}
	}
};

class VertexAttribLayout
{
private:
	std::vector<VertexAttribElement> RendererElements;
	unsigned int RendererStride;
public:
	VertexAttribLayout() :RendererStride(0) {};

	template<unsigned int T>
	void Push(unsigned int count)
	{
		RendererElements.push_back({ T,count,GL_FALSE });
		RendererStride += VertexAttribElement::GetSizeOfType(T)*count;
	}

	inline const std::vector<VertexAttribElement> GetElements() const { return RendererElements; }
	inline unsigned int GetStride() const { return RendererStride; }
};