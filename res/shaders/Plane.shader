#shader vertex
#version 460 core 

layout(location=0) in vec3 position; 
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord; //贴图坐标
layout(location=3) in mat4 model; 


out VS_OUT{
	vec2 v_texcoord;//传递（vary）给片元着色器的变量
	vec4 v_WorldNormal;
	vec4 v_WorldPosition;
	vec4 v_LightSpacePosition;
}vs_out;


layout(std140) uniform Matrices
{
	mat4 u_view;
	mat4 u_projection;
};


uniform mat4 view;
uniform mat4 projection;

void main() 
{
	vs_out.v_texcoord=texcoord;
	vs_out.v_WorldNormal=model*vec4(normalize(normal),0.0f);
	vs_out.v_WorldPosition=model*vec4(position,1.0f);
	vs_out.v_LightSpacePosition=projection*view*model*vec4(position,1.0f);
	gl_Position =u_projection*u_view*model*vec4(position,1.0f); 

}






#shader fragment
#version 460 core 

struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	//sampler2D texture_normal1;
};

out vec4 color; 

uniform Material material;
uniform sampler2D shadowmap;

in VS_OUT{
	vec2 v_texcoord;//从顶点着色器传入的变量
	vec4 v_WorldNormal;
	vec4 v_WorldPosition;
	vec4 v_LightSpacePosition;
}fs_in;


void main() 
{

    //vec3 worldLightDir = normalize(vec3(-20.0f,20.0f,20.0f)); //获取光源位置
	//vec3 lightColor = vec3(1.0f);
	//vec3 diffuseColor = fs_in.densityColor;
    //vec3 diffuse = lightColor * diffuseColor * max(0, dot(worldLightDir, normalize(fs_in.v_WorldNormal.xyz)));//+ max(0, dot(worldLight2, normalize(fs_in.v_WorldNormal.xyz)))); // 计算漫反射

	//阴影
	vec3 projcoords = fs_in.v_LightSpacePosition.xyz/fs_in.v_LightSpacePosition.w;//光源视角标准化裁剪空间坐标
	projcoords = projcoords*0.5f+0.5f;//由-1到1转为0到1
	vec2 texelSize = 5.0f / textureSize(shadowmap, 0);//采样遮挡物平均深度的矩阵大小，矩阵越大，阴影越圆润
	float shadow = 0.0f;//是否在阴影判断，1为在，0为不在
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowmap, projcoords.xy + vec2(x, y) * texelSize).r; //采样点最小深度（光源视角标准化裁剪空间坐标）
			if(projcoords.z > pcfDepth+0.00005)//采样点最小深度+0.005和采样点实际深度（光源视角标准化裁剪空间坐标）比较
			{
				shadow ++;
			}
		}    
	}

	shadow /= 9.0f;//计算该点是否在阴影（平均化）

	vec3 finalColor = vec3(1.0f - shadow);



	color =vec4(finalColor,1.0f);
}