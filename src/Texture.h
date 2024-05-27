#pragma once
#include <string>
#include "GL/glew.h"
#include "stb_image/stb_image.h"
class Texture
{
private:
	unsigned int RendererID;
	std::string FilePath;
	unsigned char* LocalBuffer;
	int Width, Height, BBP;//bits per pixel

public:
	Texture(const std::string& path);
	Texture(int width, int height);
	Texture() = default;
	~Texture();

	void Bind(unsigned int slot = 0) const;//Ä¬ÈÏË÷ÒýÎª0£¨¼´TEXCOORD0£©
	void BindComputeRead(unsigned int slot) const;
	void BindComputeWrite(unsigned int slot) const;
	void BindComputeReadWrite(unsigned int slot) const;
	void Unbind() const;

	inline int GetWidth() const { return Width; }
	inline int GetHeight() const { return Height; }

	inline GLuint GetID() const { return RendererID; }


	void TransferDataToTextureFloat(float* data);
	float* GetDataFromTextureFloat(GLuint channels);
};