#include "FrameBuffer.h"
#include "ErrorCheck.h"


FrameBuffer::FrameBuffer(unsigned int WinWidth, unsigned int WinHeight):WinWidth(WinWidth),WinHeight(WinHeight)
{
	glGenFramebuffers(1, &fbID);//创建framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbID);
}

void FrameBuffer::GenTexture2D()
{
	glGenTextures(1, &tbID);//创建贴图
	glBindTexture(GL_TEXTURE_2D, tbID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//设置纹理过滤方式（必须设置）
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//设置纹理过滤方式（必须设置）
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//设置纹理环绕方式（必须设置）
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//设置纹理环绕方式（必须设置）
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WinWidth, WinHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);//设置贴图为NULL
	glBindTexture(GL_TEXTURE_2D, 0);//解绑贴图

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbID, 0);//将该贴图设置为framebuffer的颜色附件

	glGenRenderbuffers(1, &rbID);//创建renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, rbID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WinWidth, WinHeight);//将深度缓冲和模板缓冲存入
	glBindRenderbuffer(GL_RENDERBUFFER, 0);//解绑renderbuffer

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbID);//将该renderbuffer设置为framebuffer的深度-模板附件

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)//检查framebuffer是否内容完整
		std::cout << "ERROR: Frame buffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//解绑framebuffer
}

void FrameBuffer::GenTexture2DMultiSample(unsigned int samples)
{
	mSamples = samples;
	glGenTextures(1, &tbID);//创建贴图
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tbID);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//设置纹理过滤方式（必须设置）
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//设置纹理过滤方式（必须设置）
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//设置纹理环绕方式（必须设置）
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//设置纹理环绕方式（必须设置）
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, WinWidth, WinHeight,GL_TRUE);//设置贴图为NULL
	glBindTexture(GL_TEXTURE_2D, 0);//解绑贴图

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tbID, 0);//将该贴图设置为framebuffer的颜色附件

	glGenRenderbuffers(1, &rbID);//创建renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, rbID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,GL_DEPTH24_STENCIL8, WinWidth, WinHeight);//将深度缓冲和模板缓冲存入
	glBindRenderbuffer(GL_RENDERBUFFER, 0);//解绑renderbuffer

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbID);//将该renderbuffer设置为framebuffer的深度-模板附件

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)//检查framebuffer是否内容完整
		std::cout << "ERROR: Frame buffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//解绑framebuffer
}

void FrameBuffer::GenTexture2DShadowMap()
{
	glGenTextures(1, &tbID);//创建贴图
	glBindTexture(GL_TEXTURE_2D, tbID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//设置纹理过滤方式（必须设置）
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//设置纹理过滤方式（必须设置）
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);//设置纹理环绕方式（必须设置）
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);//设置纹理环绕方式（必须设置）
	GLfloat borderColor[] = { 1.0f,1.0f,1.0f,1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);//设置border以外的深度为1.0

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WinWidth, WinHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//设置贴图为NULL
	glBindTexture(GL_TEXTURE_2D, 0);//解绑贴图

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tbID, 0);//将该贴图设置为framebuffer的深度附件

	glDrawBuffer(GL_NONE);//为了使framebuffer完整，显式声明没有颜色缓冲
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)//检查framebuffer是否内容完整
		std::cout << "ERROR: Frame buffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//解绑framebuffer
}

void FrameBuffer::GenTexture2DShadowCubeMap()
{
	glGenTextures(1, &tbID);//创建贴图
	glBindTexture(GL_TEXTURE_CUBE_MAP, tbID);
	for (GLuint i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_DEPTH_COMPONENT, WinWidth, WinHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//设置贴图为NULL
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//设置纹理过滤方式（必须设置）
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//设置纹理过滤方式（必须设置）
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);//设置纹理环绕方式（必须设置）
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);//设置纹理环绕方式（必须设置）
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);//设置纹理环绕方式（必须设置）
	GLfloat borderColor[] = { 1.0f,1.0f,1.0f,1.0f };
	glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor);//设置border以外的深度为1.0

	
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);//解绑贴图

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tbID, 0);//将该贴图设置为framebuffer的深度附件

	glDrawBuffer(GL_NONE);//为了使framebuffer完整，显式声明没有颜色缓冲
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)//检查framebuffer是否内容完整
		std::cout << "ERROR: Frame buffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//解绑framebuffer
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &fbID);
}

void FrameBuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbID);
}

void FrameBuffer::Unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Draw(Shader& ScreenShader,unsigned int vaID)
{
	
	ScreenShader.Bind();
	
	glBindVertexArray(vaID);

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, tbID);
	ScreenShader.SetUniform1i("screenTexture", 4);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	ScreenShader.Unbind();
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);

}

void FrameBuffer::GetColorAfterMSAA(unsigned int framebufferMSAA)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferMSAA);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbID);
	glBlitFramebuffer(0, 0, WinWidth, WinHeight, 0, 0, WinWidth, WinHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void FrameBuffer::ShowColorAfterMSAA(unsigned int framebufferMSAA)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferMSAA);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, WinWidth, WinHeight, 0, 0, WinWidth, WinHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void FrameBuffer::ResetWindow(unsigned int Width, unsigned int Height)
{
	WinWidth = Width;
	WinHeight = Height;
	glBindTexture(GL_TEXTURE_2D, tbID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WinWidth, WinHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);//设置贴图为NULL
	glBindTexture(GL_TEXTURE_2D, 0);//解绑贴图

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbID, 0);//将该贴图设置为framebuffer的颜色附件

	glBindRenderbuffer(GL_RENDERBUFFER, rbID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WinWidth, WinHeight);//将深度缓冲和模板缓冲存入
	glBindRenderbuffer(GL_RENDERBUFFER, 0);//解绑renderbuffer

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbID);//将该renderbuffer设置为framebuffer的深度-模板附件

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//解绑framebuffer
}
void FrameBuffer::ResetWindowMultiSample(unsigned int Width, unsigned int Height)
{
	WinWidth = Width;
	WinHeight = Height;

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tbID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mSamples, GL_RGB, WinWidth, WinHeight, GL_TRUE);//设置贴图为NULL
	glBindTexture(GL_TEXTURE_2D, 0);//解绑贴图

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tbID, 0);//将该贴图设置为framebuffer的颜色附件

	glBindRenderbuffer(GL_RENDERBUFFER, rbID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, mSamples, GL_DEPTH24_STENCIL8, WinWidth, WinHeight);//将深度缓冲和模板缓冲存入
	glBindRenderbuffer(GL_RENDERBUFFER, 0);//解绑renderbuffer

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbID);//将该renderbuffer设置为framebuffer的深度-模板附件

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//解绑framebuffer
}


unsigned int FrameBuffer::GenQuad()
{
	std::vector<SimpleVertex> quadVertices = {
		// positions // texCoords
		{glm::vec3(-1.0f,  1.0f,  0.0f),glm::vec2(0.0f, 1.0f)},
		{glm::vec3(-1.0f, -1.0f,  0.0f),glm::vec2(0.0f, 0.0f)},
		{glm::vec3(1.0f, -1.0f,  0.0f),glm::vec2(1.0f, 0.0f)},

		{glm::vec3(-1.0f,  1.0f,  0.0f),glm::vec2(0.0f, 1.0f)},
		{glm::vec3(1.0f, -1.0f,  0.0f),glm::vec2(1.0f, 0.0f)},
		 {glm::vec3(1.0f,  1.0f,  0.0f),glm::vec2(1.0f, 1.0f)}
	};
	unsigned int vaID;//VertexArray
	unsigned int vbID;

	VertexArray* va = new VertexArray(vaID);
	VertexBuffer* vb=new VertexBuffer(vbID, quadVertices.data(), (unsigned int)quadVertices.size() * sizeof(SimpleVertex));

	VertexAttribLayout layout;//创建顶点属性布局实例
	layout.Push<GL_FLOAT>(3);//填入第一个属性布局，类型为float，每个点为3维向量
	//layout.Push<GL_FLOAT>(3);//填入第二个属性布局，类型为float，每个点为3维向量
	layout.Push<GL_FLOAT>(2);//填入第三个属性布局，类型为float，每个点为2维向量

	va->AddBuffer(vbID, layout);//将所有属性布局应用于顶点缓冲区vb，并绑定在顶点数组对象va上

	va->Unbind();
	vb->Unbind();
	return vaID;
}