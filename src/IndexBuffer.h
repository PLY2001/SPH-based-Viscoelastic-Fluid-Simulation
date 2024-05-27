#pragma once

class IndexBuffer
{
private:
	unsigned int& RendererID;
	unsigned int RendererCount;
public:
	IndexBuffer(unsigned int& RendererID,const unsigned int* data, unsigned int count);
	~IndexBuffer();
	void Bind() const;
	void Unbind() const;

	inline unsigned int GetCount() const { return RendererCount; }
};