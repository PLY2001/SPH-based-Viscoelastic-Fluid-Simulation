#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(unsigned int size, unsigned int BindPoint):BindPoint(BindPoint)
{
	glGenBuffers(1, &RendererID);
	glBindBuffer(GL_UNIFORM_BUFFER, RendererID);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, RendererID, 0, 2 * sizeof(glm::mat4));

}

void UniformBuffer::Bind(std::vector<int>& shaderIDs,const char* name)
{
	for (int shaderID : shaderIDs)
	{
		unsigned int UniformBlockID = glGetUniformBlockIndex(shaderID, name);
		glUniformBlockBinding(shaderID, UniformBlockID, BindPoint);
	}
}

void UniformBuffer::SetDatamat4(unsigned int offset,unsigned int size, const glm::mat4* data)
{
	glBindBuffer(GL_UNIFORM_BUFFER, RendererID);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
}

