#include "IndexBuffer.h"
#include <GL/glew.h>

IndexBuffer::IndexBuffer(unsigned int& RendererID,const unsigned int* data, unsigned int count):RendererID(RendererID),RendererCount(count)
{
	glGenBuffers(1, &RendererID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RendererID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}
IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &RendererID);
}
void IndexBuffer::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RendererID);
}
void IndexBuffer::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}