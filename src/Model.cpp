#include "Model.h"


Model::Model(std::string path)
{
	

	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);// | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));
	processNode(scene->mRootNode, scene);
	//SetPosition();
	BoxVertex gg = GetBoxVertex();
	minBoxVertexForCompare = gg.MinVertex;
	maxBoxVertexForCompare = gg.MaxVertex;
	massCenterForCompare = (minBoxVertexForCompare + maxBoxVertexForCompare) / 2.0f;
}


Model::Model(std::vector<glm::vec2>& vertexList, int index, glm::vec3 minBoxVertex, glm::vec3 maxBoxVertex, glm::vec3 MassCenter)
{

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<myTexture> textures;


	float clampBorder = 1.001f;
	glm::vec3 maxBorderPosition = MassCenter + (maxBoxVertex - MassCenter) * clampBorder;
	glm::vec3 minBorderPosition = MassCenter + (minBoxVertex - MassCenter) * clampBorder;

	for (auto v : vertexList) {		
		glm::vec3 pos = GetVertex3D(v, index, minBorderPosition, maxBorderPosition);
		Vertex vertex;
		vertex.Position.x = pos.x;
		vertex.Position.y = pos.y;
		vertex.Position.z = pos.z;

		glm::vec3 normal = GetNormal(index);
		vertex.Normal.x = normal.x;
		vertex.Normal.y = normal.y;
		vertex.Normal.z = normal.z;

		vertices.push_back(vertex);
	}	
	int vrtexCount = vertexList.size();
	int faceCount = vrtexCount - 2;
// 	if (index == 1 || index == 2 || index == 4) {
// 		for (int i = 0; i < faceCount; i++) {
// 			indices.push_back(vrtexCount - 1);
// 			indices.push_back(i);
// 			indices.push_back(i + 1);
// 		}
// 	}
// 	else {
// 		for (int i = faceCount - 1; i > -1; i--) {
// 			indices.push_back(vrtexCount - 1);
// 			indices.push_back(i + 1);
// 			indices.push_back(i);
// 		}
// 	}
// 	
	for (int i = 0; i < 1; i = i + 1) {
		for (int j = 0; j < faceCount; j++) {
			indices.push_back(i);
			indices.push_back(j + 1);
			indices.push_back(j + 2);
		}

	}

	//for (int j = 0; j < faceCount; j++) {//     Ƿ ˳      
	//	indices.push_back(j);
	//	indices.push_back(j + 1);
	//	indices.push_back(j + 2);
	//}
	//indices.push_back(vrtexCount - 2);
	//indices.push_back(vrtexCount - 1);
	//indices.push_back(0);

	Mesh mesh(vertices, indices, textures);
	meshes.push_back(mesh);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	//     ڵ    е         еĻ   
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	//              ӽڵ  ظ   һ    
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<myTexture> textures;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		vertex.Position.x = mesh->mVertices[i].x;
		vertex.Position.y = mesh->mVertices[i].y;
		vertex.Position.z = mesh->mVertices[i].z;
	
		vertex.Normal.x = mesh->mNormals[i].x;
		vertex.Normal.y = mesh->mNormals[i].y;
		vertex.Normal.z = mesh->mNormals[i].z;

		//vertex.Tangent.x = mesh->mTangents[i].x;
		//vertex.Tangent.y = mesh->mTangents[i].y;
		//vertex.Tangent.z = mesh->mTangents[i].z;
		//vertex.Tangent = glm::vec3(0.0f,0.0f, 0.0f);
		
		if (mesh->mTextureCoords[0])
		{
			vertex.Texcoord.x = mesh->mTextureCoords[0][i].x;
			vertex.Texcoord.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.Texcoord = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
	}
	//         
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	//        
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<myTexture> diffuseMaps = loadMaterialTextures(material,aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<myTexture> specularMaps = loadMaterialTextures(material,aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		//std::vector<myTexture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		//textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	}
	//ProcessNormal(vertices);//均匀化倒角（由于例如倒角导出的面是多个面，会导致法线均匀化显示失真，弃用）
	return Mesh(vertices, indices, textures);
}

std::vector<myTexture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<myTexture> textures;
	if(mat->GetTextureCount(type)>0)
	{
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.data, str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{  
				myTexture texture;
				texture.slot = TextureFromFile(str.C_Str(), directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);
			}
		}
	}
	else
	{
		std::string path = "noise.jpg";
		aiString str = aiString(path);
		myTexture texture;
		texture.slot = TextureFromFile(str.C_Str(), directory);
		texture.type = typeName;
		
		//mat->GetTexture(type, 0, &str);
		texture.path =str.C_Str();
		textures.push_back(texture);
	}
	return textures;
}

unsigned int Model::TextureFromFile(const char* path, const std::string& directory)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		//std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

glm::mat4 Model::MatrixLerp(glm::mat4 x, glm::mat4 y, float t)
{
	glm::mat4 temp = t * y + (1 - t) * x;
	return temp;
}

bool Model::IsMoreThanVec3(glm::vec3 v1, glm::vec3 v2)
{
	return ((v1.x > v2.x) + (v1.y > v2.y) + (v1.z > v2.z));
}

glm::vec3 Model::GetVertex3D(glm::vec2 p, int i, glm::vec3 minBoxVertex, glm::vec3 maxBoxVertex)
{
	switch (i)
	{
	case 0:
		return glm::vec3(p.x, p.y, maxBoxVertex.z);
	case 1:
		return glm::vec3(p.x, p.y, minBoxVertex.z);
	case 2:
		return glm::vec3(minBoxVertex.x, p.x, p.y);
	case 3:
		return glm::vec3(maxBoxVertex.x, p.x, p.y);
	case 4:
		return glm::vec3(p.x, maxBoxVertex.y, p.y);
	case 5:
		return glm::vec3(p.x, minBoxVertex.y, p.y);
	}
}

glm::vec3 Model::GetNormal(int i)
{
	switch (i)
	{
	case 0:
		return glm::vec3(0.0f, 0.0f, 1.0f);
	case 1:
		return glm::vec3(0.0f, 0.0f, -1.0f);
	case 2:
		return glm::vec3(-1.0f, 0.0f, 0.0f);
	case 3:
		return glm::vec3(1.0f, 0.0f, 0.0f);
	case 4:
		return glm::vec3(0.0f, 1.0f, 0.0f);
	case 5:
		return glm::vec3(0.0f, -1.0f, 0.0f);
	}
}

void Model::ProcessNormal(std::vector<Vertex>& vertices)
{
	std::map<VertexKey, glm::vec3> vlist;
	VertexKey vk;
	for (const Vertex& vertex : vertices) {		
		vk.v = vertex.Position;
		if (vlist.count(vk) > 0) {
			vlist[vk] += vertex.Normal;
			continue;
		}
		else {
			vlist[vk] = glm::vec3(0) + vertex.Normal;
		}
	}

	int vertexIndex = 0;
	for (const Vertex& vertex : vertices) {
		vk.v = vertex.Position;
		vertices[vertexIndex].Normal = glm::normalize(vlist[vk]);
		vertexIndex++;
	}
	
	
}

void Model::Draw(Shader& shader)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader);
}

void Model::DrawInstanced(Shader& shader,int amount)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].DrawInstanced(shader,amount);
}

void Model::ResetToDefaultModelMatrix(float t)
{
	float totalTime = 0.1f;
	float weight = t / totalTime;
	mModelMatrix = MatrixLerp(mModelMatrix, defaultModelMatrix, weight);//从mModelMatrix向defaultModelMatrix平滑过渡，效果很赞
}

void Model::SetModelMatrixPosition(glm::vec3 Pos)
{
	mModelMatrix = glm::translate(mModelMatrix, Pos);
}

void Model::SetModelMatrixRotation(float Radians, glm::vec3 Axis)
{
	mModelMatrix = glm::rotate(mModelMatrix, Radians, Axis);
}

void Model::SetModelMatrixScale(glm::vec3 Scale)
{
	mModelMatrix = glm::scale(mModelMatrix, Scale);
}

void Model::SetModelMatrix(glm::mat4 matrix)
{
	mModelMatrix = matrix;
}

float Model::GetNormalizeScale(glm::vec3 MassCenter)
{
	float maxDis = 0;
	for (auto mesh : meshes) {
		for (auto vertex : mesh.vertices) {
			float dis = glm::distance(vertex.Position, MassCenter);
			maxDis = dis > maxDis ? dis : maxDis;
		}		
	}
	
	return 50.0f / maxDis;
}

BorderVertexList Model::GetBorderVertexList(glm::vec3 minBoxVertex, glm::vec3 maxBoxVertex, glm::vec3 MassCenter)
{
	BorderVertexList borderVertexList;

	float clampBorder = 0.99f;
	glm::vec3 maxBorderPosition = MassCenter + (maxBoxVertex - MassCenter) * clampBorder;
	glm::vec3 minBorderPosition = MassCenter + (minBoxVertex - MassCenter) * clampBorder;
	
	std::map<VertexKey, int> vlist;
	for (auto mesh : meshes) {
		for (auto vertex : mesh.vertices) {
			VertexKey vk;
			vk.v = vertex.Position;
			if (vlist.count(vk) > 0) {
				continue;
			}
			else {
				vlist[vk] = 1;
			}
			if (vertex.Position.z > maxBorderPosition.z) {
				borderVertexList.VertexList[0].push_back(vertex.Position);
			}
			if (vertex.Position.z < minBorderPosition.z) {
				borderVertexList.VertexList[1].push_back(vertex.Position);
			}
			if (vertex.Position.x < minBorderPosition.x) {
				borderVertexList.VertexList[2].push_back(vertex.Position);
			}
			if (vertex.Position.x > maxBorderPosition.x) {
				borderVertexList.VertexList[3].push_back(vertex.Position);
			}
			if (vertex.Position.y > maxBorderPosition.y) {
				borderVertexList.VertexList[4].push_back(vertex.Position);
			}
			if (vertex.Position.y < minBorderPosition.y) {
				borderVertexList.VertexList[5].push_back(vertex.Position);
			}
		}
	}
	return borderVertexList;
}

BoxVertex Model::GetBoxVertex()
{
	BoxVertex result;
	glm::vec3 minBoxVertex = meshes[0].vertices[0].Position;
	glm::vec3 maxBoxVertex = meshes[0].vertices[0].Position;
	for (auto mesh : meshes) {
		for (auto vertex : mesh.vertices) {
			minBoxVertex.x = minBoxVertex.x < vertex.Position.x ? minBoxVertex.x : vertex.Position.x;
			minBoxVertex.y = minBoxVertex.y < vertex.Position.y ? minBoxVertex.y : vertex.Position.y;
			minBoxVertex.z = minBoxVertex.z < vertex.Position.z ? minBoxVertex.z : vertex.Position.z;
			maxBoxVertex.x = maxBoxVertex.x > vertex.Position.x ? maxBoxVertex.x : vertex.Position.x;
			maxBoxVertex.y = maxBoxVertex.y > vertex.Position.y ? maxBoxVertex.y : vertex.Position.y;
			maxBoxVertex.z = maxBoxVertex.z > vertex.Position.z ? maxBoxVertex.z : vertex.Position.z;
		}
	}
	result.MinVertex = minBoxVertex;
	result.MaxVertex = maxBoxVertex;
	return result;
}

std::vector<glm::vec3> Model::GetVertexList()
{
	std::vector<glm::vec3> result;
	std::map<VertexKey, int> vlist;
	for (auto mesh : meshes) {
		for (auto vertex : mesh.vertices) {
			VertexKey vk;
			vk.v = vertex.Position;
			if (vlist.count(vk) > 0) {
				continue;
			}
			else {
				vlist[vk] = 1;
			}
			result.push_back(vertex.Position);
		}
	}
	return result;
}

