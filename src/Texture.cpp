#include "Texture.h"


Texture::Texture(const std::string& path):RendererID(0),FilePath(path),LocalBuffer(nullptr),Width(0),Height(0),BBP(0)
{
	//stbi_set_flip_vertically_on_load(1);//��ֱ��ת������ΪOpenGLͼƬԭ�������½ǣ���pngͼƬ��ȡ�Ǵ����Ͻǿ�ʼ��
	LocalBuffer = stbi_load(path.c_str(), &Width, &Height, &BBP, 4);//ʹ��stb_image�⺯������ͼƬ��

	glGenTextures(1, &RendererID);
	glBindTexture(GL_TEXTURE_2D, RendererID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//����������˷�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//����������˷�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//���������Ʒ�ʽ���������ã�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//���������Ʒ�ʽ���������ã�

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, LocalBuffer);//����ȡ��ͼƬ������ͼ
	glBindTexture(GL_TEXTURE_2D, 0);

	if (LocalBuffer)//������ͼ�ɹ��󣬿��������ȡ��ͼƬ����
		stbi_image_free(LocalBuffer);
}

Texture::Texture(int width, int height):RendererID(0), LocalBuffer(nullptr), Width(width), Height(height), BBP(0)
{
	glGenTextures(1, &RendererID);
	glBindTexture(GL_TEXTURE_2D, RendererID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);

}

Texture::~Texture()
{
	glDeleteTextures(1, &RendererID);
}

void Texture::Bind(unsigned int slot) const
{
	glActiveTexture(GL_TEXTURE0+slot);//������ǰ��Ҫ�ȼ����Ӧ������Ԫ������Ϊ��0+slot��������Ϊ0�ĵ�ԪOpenGL�ǻ��Զ�����ģ�
	glBindTexture(GL_TEXTURE_2D, RendererID);
}

void Texture::BindComputeRead(unsigned int slot) const
{
	glBindImageTexture(slot, RendererID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

}

void Texture::BindComputeWrite(unsigned int slot) const
{
	glBindImageTexture(slot, RendererID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

}

void Texture::BindComputeReadWrite(unsigned int slot) const
{
	glBindImageTexture(slot, RendererID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::TransferDataToTextureFloat(float* data)
{
	glBindTexture(GL_TEXTURE_2D, RendererID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, GL_RGBA, GL_FLOAT, data);
}

float* Texture::GetDataFromTextureFloat(GLuint channels)
{
	float* data = new float[Width * Height * channels];
	glBindTexture(GL_TEXTURE_2D, RendererID);
	if (channels == 1) glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data);
	if (channels == 3) glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, data);
	if (channels == 4) glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return data;
	

}

