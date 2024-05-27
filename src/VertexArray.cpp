#include "VertexArray.h"
#include "GL/glew.h"

VertexArray::VertexArray(unsigned int& RendererID):RendererID(RendererID)
{
	glGenVertexArrays(1, &RendererID);
	glBindVertexArray(RendererID);
}
VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &RendererID);
}

void VertexArray::AddBuffer(const unsigned int& vbID, const VertexAttribLayout& layout)
{
	Bind();
	glBindBuffer(GL_ARRAY_BUFFER, vbID);
	const auto& elements = layout.GetElements();
	unsigned int offset = 0;
	for (int i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)offset);
		offset += element.count * VertexAttribElement::GetSizeOfType(element.type);
	}
	
}

void VertexArray::Bind() const
{
	glBindVertexArray(RendererID);

}

void VertexArray::Unbind() const
{
	glBindVertexArray(0);
}