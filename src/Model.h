#pragma once
#include "Mesh.h"
#include <iostream>
#include "stb_image/stb_image.h"
#include "GL/glew.h"
#include <map>

class VertexKey //用于代替glm::vec3作为map的key，其中重载了操作符<
{
public:
	glm::vec3 v;
	bool operator < (const VertexKey& vk) const
	{
		return v.x < vk.v.x ? true : (v.x > vk.v.x ? false : (v.y < vk.v.y ? true : (v.y > vk.v.y ? false : (v.z < vk.v.z))));
		//if (v.x < vk.v.x) {
		//	return true;
		//}
		//else if (v.x > vk.v.x) {
		//	return false;
		//}
		//else {
		//	if (v.y < vk.v.y) {
		//		return true;
		//	}
		//	else if (v.y > vk.v.y) {
		//		return false;
		//	}
		//	else {
		//		return v.z < vk.v.z;
		//	}
		//}
	}
};

struct BorderVertexList {
	std::vector<glm::vec3> VertexList[6];//前后左右上下
};

struct BorderVertexList2D {
	std::vector<glm::vec2> VertexList[6];//前后左右上下
};

struct BoxVertex {
	glm::vec3 MinVertex = glm::vec3(0);
	glm::vec3 MaxVertex = glm::vec3(0);
};

class Model
{
private:
	glm::vec3 Pos;
	std::string directory;
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<myTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	unsigned int TextureFromFile(const char* path, const std::string& directory);
	std::vector<myTexture> textures_loaded;
	glm::mat4 mModelMatrix;
	glm::mat4 defaultModelMatrix;
	glm::mat4 MatrixLerp(glm::mat4 x, glm::mat4 y, float t);
	bool IsMoreThanVec3(glm::vec3 v1, glm::vec3 v2);
	glm::vec3 GetVertex3D(glm::vec2 p, int i, glm::vec3 minBoxVertex, glm::vec3 maxBoxVertex);
	glm::vec3 GetNormal(int i);
	void ProcessNormal(std::vector<Vertex>& vertices);

	/////////////////用于算法比较//////////
	glm::vec3 massCenterForCompare;
	glm::vec3 minBoxVertexForCompare;
	glm::vec3 maxBoxVertexForCompare;
	/////////////////////////////////////////

public:
	Model(std::string path);
	Model(std::vector<glm::vec2>& vertexList, int index, glm::vec3 minBoxVertex, glm::vec3 maxBoxVertex, glm::vec3 massCenter);
	Model() = default;
	void Draw(Shader& shader);
	void DrawInstanced(Shader& shader, int amount);
	std::vector<Mesh> meshes;
	inline glm::mat4& GetModelMatrix() { return mModelMatrix; };
	inline glm::mat4& GetDefaultModelMatrix() { return defaultModelMatrix; };
	inline void SetDefaultModelMatrix() { defaultModelMatrix = mModelMatrix; };
	void ResetToDefaultModelMatrix(float t);
	void SetModelMatrixPosition(glm::vec3 Pos);
	void SetModelMatrixRotation(float Radians, glm::vec3 Axis);
	void SetModelMatrixScale(glm::vec3 Scale);
	void SetModelMatrix(glm::mat4 matrix);
	float GetNormalizeScale(glm::vec3 MassCenter);
	BorderVertexList GetBorderVertexList(glm::vec3 minBoxVertex, glm::vec3 maxBoxVertex, glm::vec3 MassCenter);
	BoxVertex GetBoxVertex();
	std::vector<glm::vec3> GetVertexList();

	/////////////////用于算法比较//////////
	inline glm::vec3& GetMassCenterForCompare() { return massCenterForCompare; }
	inline glm::vec3& GetMinBoxForCompare() { return minBoxVertexForCompare; };
	inline glm::vec3& GetMaxBoxForCompare() { return maxBoxVertexForCompare; };
	/////////////////////////////////////////
};