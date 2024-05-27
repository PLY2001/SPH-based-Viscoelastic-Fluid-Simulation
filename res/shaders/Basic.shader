#shader vertex
#version 460 core 

layout(location=0) in vec3 position; 
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord; //贴图坐标
layout(location=3) in mat4 model; 
layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D parameter_image;
layout (rgba32f, binding = 2) uniform image2D density_image;


out VS_OUT{
	vec2 v_texcoord;//传递（vary）给片元着色器的变量
	vec4 v_WorldNormal;
	vec4 v_WorldPosition;
	vec3 velocityColor;
	vec3 densityColor;
	vec4 v_LightSpacePosition;
}vs_out;


layout(std140) uniform Matrices
{
	mat4 u_view;
	mat4 u_projection;
};

ivec3 PointDim = ivec3(imageLoad(parameter_image,ivec2(4,0)).xyz);//获取球集合图片大小
float TargetDensity = imageLoad(parameter_image,ivec2(2,0)).x;//获取目标密度
uniform float MaxVelocity;
uniform mat4 view;
uniform mat4 projection;


void main() 
{ 
	//缩放
    float s = 2.0;
    mat4 m = mat4(
        s,  0.0, 0.0, 0.0,
        0.0, s,  0.0, 0.0,
        0.0, 0.0, s, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

	//计算索引
	int uv_xy_i = gl_InstanceID % (PointDim.x * PointDim.y);
	int uv_z = gl_InstanceID / (PointDim.x * PointDim.y);
	ivec2 uv = ivec2(uv_xy_i,uv_z);
	ivec2 uv1 = ivec2(uv_xy_i,uv_z + PointDim.z);

	//计算密度颜色
	float density = imageLoad(density_image,uv).x;
	float densityCut1 = TargetDensity * 2.0f/3.0f;
	float densityCut2 = TargetDensity * 2.0f/3.0f*2.0f;
	float densityR = -1.0f + density/densityCut1;
	float densityG = density>400.0f*0.5f? 3.0f - density*2.0f/densityCut2 : density/densityCut1;
	float densityB = 2.0f - density*2.0f/densityCut2;
	vs_out.densityColor = vec3(densityR,densityG,densityB);

	//计算速度颜色
	vec3 velocity = vec3(imageLoad(input_image,uv1).xyz);
	float velocitySize = velocity.x*velocity.x + velocity.y*velocity.y + velocity.z*velocity.z;
	float velocitySizeCut1 = MaxVelocity/3.0f;
	float velocitySizeCut2 = MaxVelocity/3.0f*2.0f;
	float velocityR = -1.0f + velocitySize/velocitySizeCut1;
	float velocityG = velocitySize>MaxVelocity*0.5f? 3.0f - velocitySize*2.0f/velocitySizeCut2 : velocitySize/velocitySizeCut1;
	float velocityB = 2.0f - velocitySize*2.0f/velocitySizeCut2;
	vs_out.velocityColor = vec3(velocityR,velocityG,velocityB);

	//坐标偏移
	vec3 posBias = vec3(imageLoad(input_image,uv).xyz);
	vec4 realPos = m*vec4(position,1.0f) + vec4(posBias,0.0f);

	//灯光视角中的坐标
	vs_out.v_LightSpacePosition=projection*view*model*realPos;

	vs_out.v_texcoord=texcoord;
	vs_out.v_WorldNormal=model*vec4(normalize(normal),0.0f);
	vs_out.v_WorldPosition=model*realPos;
	gl_Position =u_projection*u_view*model*realPos; 

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
uniform int VisualMode;

in VS_OUT{
	vec2 v_texcoord;//从顶点着色器传入的变量
	vec4 v_WorldNormal;
	vec4 v_WorldPosition;
	vec3 velocityColor;
	vec3 densityColor;
	vec4 v_LightSpacePosition;
}fs_in;


void main() 
{

    
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

	//漫反射
	vec3 worldLightDir = normalize(vec3(20.0f,20.0f,0.0f)); //获取光源位置
	vec3 lightColor = vec3(1.0f - shadow);
	vec3 diffuseColor = VisualMode > 0? fs_in.velocityColor : fs_in.densityColor;
    vec3 diffuse = lightColor * diffuseColor * max(0, dot(worldLightDir, normalize(fs_in.v_WorldNormal.xyz)));//+ max(0, dot(worldLight2, normalize(fs_in.v_WorldNormal.xyz)))); // 计算漫反射


	vec3 finalColor = diffuse;// * vec3(1.0f - shadow);



	color =vec4(finalColor,1.0f);
}