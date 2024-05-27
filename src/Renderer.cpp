#include "Renderer.h"

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib,const Shader& shader) const
{
	shader.Bind();



	va.Bind();//绑定顶点数组对象
	ib.Bind();//绑定顶点索引缓冲区
	glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::ClearColor(float ColorR, float ColorG, float ColorB, float ColorA) const
{
	glClearColor(ColorR,ColorG,ColorB,ColorA);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::ClearDepth() const
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::CullFace(int mode)
{
	if(mode == 0)
		glCullFace(GL_BACK);
	else
		glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
}
