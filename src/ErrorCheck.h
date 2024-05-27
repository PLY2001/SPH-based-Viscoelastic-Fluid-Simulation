#pragma once
#include "GL/glew.h"
#include <iostream>
static void GLClearError()
{
	while (glGetError() != GL_NO_ERROR)
	{

	}
}

static GLenum GLCheckError()
{
	return glGetError();
}
