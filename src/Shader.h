#pragma once
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>

struct ShaderSource
{
	std::string vertexShader;
	std::string geometryShader;
	std::string fragmentShader;
	std::string computeShader;
};

class Shader
{
private:
	std::string RendererFilePath;
	std::unordered_map<std::string, int> RendererUniformLocation;//关联容器，是以字符串为key，整型为value的map
	int geo = 0;
	int compute = 0;
public:
	unsigned int RendererID;
	Shader(const std::string& filepath);
	~Shader();

	void Bind() const;
	void BindCompute(int a,int b,int c) const;
	void Unbind() const;
	void SetUniform1f(const std::string& name, float v0);
	void SetUniform1fCompute(const unsigned int slot, float v0);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform4f(const std::string& name,float v0,float v1,float v2,float v3);
	void SetUniform1i(const std::string& name,int value);
	void SetUniformMat4(const std::string& name, glm::mat4& value);

private:
	int GetUniformLocation(const std::string& name);
	ShaderSource ParseSahder(const std::string& filepath);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& geometryShader, const std::string& fragmentShader, const std::string& computeShader);
};