#shader vertex
#version 460 core 

layout(location=0) in vec3 position; 

layout(std140) uniform Matrices
{
	mat4 u_view;
	mat4 u_projection;
};

void main() 
{ 
	gl_Position = u_projection*u_view*vec4(position,1.0f);
}


#shader fragment
#version 460 core 


out vec4 color; 

void main() 
{
	color = vec4(0.0f,0.0f,0.0f,1.0f);
}