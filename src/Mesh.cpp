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

	VertexAttribLayout layout;//�����������Բ���ʵ��
	layout.Push<GL_FLOAT>(3);//�����һ�����Բ��֣�����Ϊfloat��ÿ����Ϊ3ά����
	layout.Push<GL_FLOAT>(3);//����ڶ������Բ��֣�����Ϊfloat��ÿ����Ϊ2ά����
	//layout.Push<GL_FLOAT>(3);//����ڶ������Բ��֣�����Ϊfloat��ÿ����Ϊ2ά����
	layout.Push<GL_FLOAT>(2);//������������Բ��֣�����Ϊfloat��ÿ����Ϊ2ά����

	va->AddBuffer(vbID, layout);//���������Բ���Ӧ���ڶ��㻺����vb�������ڶ����������va��
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
		glActiveTexture(GL_TEXTURE0 + i); //�ڰ�֮ǰ������Ӧ������Ԫ
		//��ȡ�������,diffuse_textureN �е� N
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
		glActiveTexture(GL_TEXTURE0 + i); //�ڰ�֮ǰ������Ӧ������Ԫ
		//��ȡ�������,diffuse_textureN �е� N
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


