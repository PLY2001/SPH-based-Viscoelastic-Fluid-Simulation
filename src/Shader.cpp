#include "Shader.h"


Shader::Shader(const std::string& filepath):RendererFilePath(filepath),RendererID(0)
{
	ShaderSource source = ParseSahder(filepath);
	RendererID= CreateShader(source.vertexShader, source.geometryShader, source.fragmentShader, source.computeShader);

	
	
}
Shader::~Shader()
{
	glDeleteProgram(RendererID);
}

void Shader::Bind() const
{
	glUseProgram(RendererID);
}

void Shader::BindCompute(int a, int b, int c) const
{
	glUseProgram(RendererID);
	glDispatchCompute(a, b, c);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);//让渲染管线的其他pass等待compute shader执行完毕
}

void Shader::Unbind() const
{
	glUseProgram(0);
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& geometryShader, const std::string& fragmentShader, const std::string& computeShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs;
	unsigned int gs;
	unsigned int fs;
	unsigned int cs;
	if (compute > 0) {
		cs = CompileShader(GL_COMPUTE_SHADER, computeShader);
		GLint success;
		glGetShaderiv(cs, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar infoLog[512];
			glGetShaderInfoLog(cs, 512, NULL, infoLog);
			std::cout<< RendererFilePath << ": Compute shader compilation failed: "<< infoLog<<std::endl;
		}
		glAttachShader(program, cs);
	}
	else {
		vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
		GLint success;
		glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar infoLog[512];
			glGetShaderInfoLog(vs, 512, NULL, infoLog);
			std::cout << RendererFilePath << ": Vertex shader compilation failed: " << infoLog << std::endl;
		}
		if (geo > 0) {
			gs = CompileShader(GL_GEOMETRY_SHADER, geometryShader);
			glGetShaderiv(gs, GL_COMPILE_STATUS, &success);
			if (!success) {
				GLchar infoLog[512];
				glGetShaderInfoLog(gs, 512, NULL, infoLog);
				std::cout << RendererFilePath << ": Geometry shader compilation failed: " << infoLog << std::endl;
			}
		}
		fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
		glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar infoLog[512];
			glGetShaderInfoLog(fs, 512, NULL, infoLog);
			std::cout << RendererFilePath << ": Fragment shader compilation failed: " << infoLog << std::endl;
		}


		glAttachShader(program, vs);
		if (geo > 0)
			glAttachShader(program, gs);
		glAttachShader(program, fs);
	}


	glLinkProgram(program);
	glValidateProgram(program);

	return program;
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);
	return id;

}

ShaderSource Shader::ParseSahder(const std::string& filepath)
{
	std::ifstream source(filepath);
	std::stringstream ss[4];
	std::string line;
	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, GEOMETRY = 1, FRAGMENT = 2, COMPUTE = 3
	};
	ShaderType type = ShaderType::NONE;
	while (getline(source, line))
	{
		if (line.find("#shader") != -1)
		{
			if (line.find("vertex") != -1)
				type = ShaderType::VERTEX;
			else if (line.find("geometry") != -1)
			{
				type = ShaderType::GEOMETRY;
				geo = 1;
			}
			else if (line.find("fragment") != -1)
				type = ShaderType::FRAGMENT;
			else if (line.find("compute") != -1) {
				type = ShaderType::COMPUTE;
				compute = 1;
			}
		}
		else
		{
			ss[(int)type] << line << '\n';
		}

	}
	return { ss[0].str(),ss[1].str(),ss[2].str(),ss[3].str() };
}

void Shader::SetUniform1f(const std::string& name, float v0)
{
	glUniform1f(GetUniformLocation(name), v0);
}


void Shader::SetUniform1fCompute(const unsigned int slot, float v0)
{
	glUniform1f(slot, v0);
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
	glUniform3f(GetUniformLocation(name), v0, v1, v2);
}


void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
	glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}

void Shader::SetUniform1i(const std::string& name,int value)
{
	glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniformMat4(const std::string& name, glm::mat4& value)
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}


int Shader::GetUniformLocation(const std::string& name)
{
	if (RendererUniformLocation.find(name) != RendererUniformLocation.end())
		return RendererUniformLocation[name];
	else
	{
		RendererUniformLocation[name] = glGetUniformLocation(RendererID, name.c_str());
		return RendererUniformLocation[name];
	}
		
}