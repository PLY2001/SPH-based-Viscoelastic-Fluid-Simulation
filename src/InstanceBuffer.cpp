#include "InstanceBuffer.h"

InstanceBuffer::InstanceBuffer(unsigned int size, const glm::mat4* data)
{
	glGenBuffers(1, &RendererID);
	glBindBuffer(GL_ARRAY_BUFFER, RendererID);
	//glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);//Ԥ��ռ� release����
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InstanceBuffer::AddInstanceBuffermat4(unsigned int vaID, unsigned int AttribIndex)
{
	glBindVertexArray(vaID);

	glBindBuffer(GL_ARRAY_BUFFER, RendererID); //��ʵ�������黺�����Ͷ���������������һ��
	//mat4���ͣ��൱�������ڴ��е�4��vec4����
	glEnableVertexAttribArray(AttribIndex);
	glVertexAttribPointer(AttribIndex, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(AttribIndex + 1);
	glVertexAttribPointer(AttribIndex + 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(AttribIndex + 2);
	glVertexAttribPointer(AttribIndex + 2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(sizeof(glm::vec4) * 2));
	glEnableVertexAttribArray(AttribIndex + 3);
	glVertexAttribPointer(AttribIndex + 3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(sizeof(glm::vec4) * 3));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glVertexAttribDivisor(AttribIndex, 1); // tell OpenGL this is an instanced vertex attribute.
	glVertexAttribDivisor(AttribIndex + 1, 1); // tell OpenGL this is an instanced vertex attribute.
	glVertexAttribDivisor(AttribIndex + 2, 1); // tell OpenGL this is an instanced vertex attribute.
	glVertexAttribDivisor(AttribIndex + 3, 1); // tell OpenGL this is an instanced vertex attribute.

	glBindVertexArray(0);

}

void InstanceBuffer::SetDatamat4(unsigned int size, const glm::mat4* data)
{
	glBindBuffer(GL_ARRAY_BUFFER, RendererID);
	glBufferData(GL_ARRAY_BUFFER, size,data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

