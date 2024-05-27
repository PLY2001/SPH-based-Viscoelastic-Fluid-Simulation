#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
class InstanceBuffer
{
private:
	unsigned int RendererID;
public:
	InstanceBuffer(unsigned int size,const glm::mat4* data);
	InstanceBuffer() = default;
	void AddInstanceBuffermat4(unsigned int vaID, unsigned int AttribIndex);
	void SetDatamat4(unsigned int size, const glm::mat4* data);
};