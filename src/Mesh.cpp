#include "Mesh.h"



Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<myTexture> textures):
	vertices(vertices),indices(indices),textures(textures)
{
	SetMesh();
}

void Mesh::SetMesh()
{
	VertexArray* va=new VertexArray(vaID);
	VertexBuffer* vb=new VertexBuffer(vbID, vertices.data(), (unsigned int)vertices.size() * sizeof(Vertex));

	VertexAttribLayout layout;//创建顶点属性布局实例
	layout.Push<GL_FLOAT>(3);//填入第一个属性布局，类型为float，每个点为3维向量
	layout.Push<GL_FLOAT>(3);//填入第二个属性布局，类型为float，每个点为2维向量
	//layout.Push<GL_FLOAT>(3);//填入第二个属性布局，类型为float，每个点为2维向量
	layout.Push<GL_FLOAT>(2);//填入第三个属性布局，类型为float，每个点为2维向量

	va->AddBuffer(vbID, layout);//将所有属性布局应用于顶点缓冲区vb，并绑定在顶点数组对象va上
	IndexBuffer* ib=new IndexBuffer(ibID,indices.data(), (unsigned int)indices.size());

	va->Unbind();
	vb->Unbind();
	ib->Unbind();
}

void Mesh::Draw(Shader& shader)
{
	shader.Bind();
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	//unsigned int normalNr = 1;
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); //在绑定之前激活相应的纹理单元
		//获取纹理序号,diffuse_textureN 中的 N
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);
		//else if (name == "texture_normal")
		//number = std::to_string(normalNr++);
		
		shader.SetUniform1i(("material." + name + number).c_str(), i);
	    

		glBindTexture(GL_TEXTURE_2D, textures[i].slot);
	}
	glActiveTexture(GL_TEXTURE0);

	
	glBindVertexArray(vaID);
	glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void Mesh::DrawInstanced(Shader& shader,int amount)
{
	shader.Bind();
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	//unsigned int normalNr = 1;
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); //在绑定之前激活相应的纹理单元
		//获取纹理序号,diffuse_textureN 中的 N
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);
		//else if (name == "texture_normal")
		//	number = std::to_string(normalNr++);


		shader.SetUniform1i(("material." + name + number).c_str(), i);


		glBindTexture(GL_TEXTURE_2D, textures[i].slot);
	}
	glActiveTexture(GL_TEXTURE0);


	glBindVertexArray(vaID);
	glDrawElementsInstanced(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, nullptr,amount);
	glBindVertexArray(0);

}


