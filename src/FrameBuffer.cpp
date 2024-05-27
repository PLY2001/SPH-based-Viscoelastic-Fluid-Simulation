#include "FrameBuffer.h"
#include "ErrorCheck.h"


FrameBuffer::FrameBuffer(unsigned int WinWidth, unsigned int WinHeight):WinWidth(WinWidth),WinHeight(WinHeight)
{
	glGenFramebuffers(1, &fbID);//����framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbID);
}

void FrameBuffer::GenTexture2D()
{
	glGenTextures(1, &tbID);//������ͼ
	glBindTexture(GL_TEXTURE_2D, tbID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//����������˷�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//����������˷�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//���������Ʒ�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//���������Ʒ�ʽ���������ã�
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WinWidth, WinHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);//������ͼΪNULL
	glBindTexture(GL_TEXTURE_2D, 0);//�����ͼ

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbID, 0);//������ͼ����Ϊframebuffer����ɫ����

	glGenRenderbuffers(1, &rbID);//����renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, rbID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WinWidth, WinHeight);//����Ȼ����ģ�建�����
	glBindRenderbuffer(GL_RENDERBUFFER, 0);//���renderbuffer

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbID);//����renderbuffer����Ϊframebuffer�����-ģ�帽��

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)//���framebuffer�Ƿ���������
		std::cout << "ERROR: Frame buffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//���framebuffer
}

void FrameBuffer::GenTexture2DMultiSample(unsigned int samples)
{
	mSamples = samples;
	glGenTextures(1, &tbID);//������ͼ
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tbID);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//����������˷�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//����������˷�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//���������Ʒ�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//���������Ʒ�ʽ���������ã�
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, WinWidth, WinHeight,GL_TRUE);//������ͼΪNULL
	glBindTexture(GL_TEXTURE_2D, 0);//�����ͼ

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tbID, 0);//������ͼ����Ϊframebuffer����ɫ����

	glGenRenderbuffers(1, &rbID);//����renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, rbID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,GL_DEPTH24_STENCIL8, WinWidth, WinHeight);//����Ȼ����ģ�建�����
	glBindRenderbuffer(GL_RENDERBUFFER, 0);//���renderbuffer

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbID);//����renderbuffer����Ϊframebuffer�����-ģ�帽��

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)//���framebuffer�Ƿ���������
		std::cout << "ERROR: Frame buffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//���framebuffer
}

void FrameBuffer::GenTexture2DShadowMap()
{
	glGenTextures(1, &tbID);//������ͼ
	glBindTexture(GL_TEXTURE_2D, tbID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//����������˷�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//����������˷�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);//���������Ʒ�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);//���������Ʒ�ʽ���������ã�
	GLfloat borderColor[] = { 1.0f,1.0f,1.0f,1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);//����border��������Ϊ1.0

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WinWidth, WinHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//������ͼΪNULL
	glBindTexture(GL_TEXTURE_2D, 0);//�����ͼ

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tbID, 0);//������ͼ����Ϊframebuffer����ȸ���

	glDrawBuffer(GL_NONE);//Ϊ��ʹframebuffer��������ʽ����û����ɫ����
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)//���framebuffer�Ƿ���������
		std::cout << "ERROR: Frame buffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//���framebuffer
}

void FrameBuffer::GenTexture2DShadowCubeMap()
{
	glGenTextures(1, &tbID);//������ͼ
	glBindTexture(GL_TEXTURE_CUBE_MAP, tbID);
	for (GLuint i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_DEPTH_COMPONENT, WinWidth, WinHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//������ͼΪNULL
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//����������˷�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//����������˷�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);//���������Ʒ�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);//���������Ʒ�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);//���������Ʒ�ʽ���������ã�
	GLfloat borderColor[] = { 1.0f,1.0f,1.0f,1.0f };
	glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor);//����border��������Ϊ1.0

	
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);//�����ͼ

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tbID, 0);//������ͼ����Ϊframebuffer����ȸ���

	glDrawBuffer(GL_NONE);//Ϊ��ʹframebuffer��������ʽ����û����ɫ����
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)//���framebuffer�Ƿ���������
		std::cout << "ERROR: Frame buffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//���framebuffer
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WinWidth, WinHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);//������ͼΪNULL
	glBindTexture(GL_TEXTURE_2D, 0);//�����ͼ

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbID, 0);//������ͼ����Ϊframebuffer����ɫ����

	glBindRenderbuffer(GL_RENDERBUFFER, rbID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WinWidth, WinHeight);//����Ȼ����ģ�建�����
	glBindRenderbuffer(GL_RENDERBUFFER, 0);//���renderbuffer

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbID);//����renderbuffer����Ϊframebuffer�����-ģ�帽��

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//���framebuffer
}
void FrameBuffer::ResetWindowMultiSample(unsigned int Width, unsigned int Height)
{
	WinWidth = Width;
	WinHeight = Height;

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tbID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mSamples, GL_RGB, WinWidth, WinHeight, GL_TRUE);//������ͼΪNULL
	glBindTexture(GL_TEXTURE_2D, 0);//�����ͼ

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tbID, 0);//������ͼ����Ϊframebuffer����ɫ����

	glBindRenderbuffer(GL_RENDERBUFFER, rbID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, mSamples, GL_DEPTH24_STENCIL8, WinWidth, WinHeight);//����Ȼ����ģ�建�����
	glBindRenderbuffer(GL_RENDERBUFFER, 0);//���renderbuffer

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbID);//����renderbuffer����Ϊframebuffer�����-ģ�帽��

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//���framebuffer
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

	VertexAttribLayout layout;//�����������Բ���ʵ��
	layout.Push<GL_FLOAT>(3);//�����һ�����Բ��֣�����Ϊfloat��ÿ����Ϊ3ά����
	//layout.Push<GL_FLOAT>(3);//����ڶ������Բ��֣�����Ϊfloat��ÿ����Ϊ3ά����
	layout.Push<GL_FLOAT>(2);//������������Բ��֣�����Ϊfloat��ÿ����Ϊ2ά����

	va->AddBuffer(vbID, layout);//���������Բ���Ӧ���ڶ��㻺����vb�������ڶ����������va��

	va->Unbind();
	vb->Unbind();
	return vaID;
}