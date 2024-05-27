#pragma once
#include <GL/glew.h>
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>

class UniformBuffer
{
private:
	unsigned int RendererID;
	unsigned int BindPoint;
public:
	UniformBuffer(unsigned int size, unsigned int BindPoint);
	void Bind(std::vector<int>& shaderIDs, const char* name);
	void SetDatamat4(unsigned int offset, unsigned int size, const glm::mat4* data);
};