#shader vertex
#version 330 core 

layout(location=0) in vec3 position; 
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord; //贴图坐标
layout(location=3) in mat4 model; 

out VS_OUT{
	vec2 v_texcoord;//传递（vary）给片元着色器的变量
	vec4 v_WorldNormal;
	//vec4 v_ViewNormal;
	vec4 v_WorldPosition;
}vs_out;


layout(std140) uniform Matrices
{
	mat4 u_view;
	mat4 u_projection;
};


void main() 
{ 
	vs_out.v_texcoord=texcoord;
	vs_out.v_WorldNormal=model*vec4(normalize(normal),0.0f);
	//vs_out.v_ViewNormal=u_view*model*vec4(normalize(normal),0.0f);
	vs_out.v_WorldPosition=model*vec4(position,1.0f);
	gl_Position =u_projection*u_view*model*vec4(position,1.0f); 

}






#shader fragment
#version 330 core 

struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	//sampler2D texture_normal1;
};

out vec4 color; 

uniform Material material;
uniform vec3 cameraFront;
uniform int toCullBack;

in VS_OUT{
	vec2 v_texcoord;//从顶点着色器传入的变量
	vec4 v_WorldNormal;
	vec4 v_WorldPosition;
}fs_in;


void main() 
{
	//color =vec4(1.0f,0.0f,0.0f,0.6f);//*(1.0f-shadowColor) //texColor;//u_color;//vec4(0.2,0.7,0.3,1.0); 
	vec3 normal = normalize(fs_in.v_WorldNormal.xyz);
	float alpha = dot(normal,cameraFront) > 0.0f? 0.0f : 0.6f;
	alpha = toCullBack > 0? alpha : 0.6f;
	color =vec4(1.0f,0.0f,0.0f,alpha);
}