#shader vertex
#version 460 core 

layout(location=0) in vec3 position; 

layout(location=3) in mat4 model; 

layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D parameter_image;

uniform mat4 view;
uniform mat4 projection;
ivec3 PointDim = ivec3(imageLoad(parameter_image,ivec2(4,0)).xyz);//获取球集合图片大小


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

    //坐标偏移
	vec3 posBias = vec3(imageLoad(input_image,uv).xyz);
	vec4 realPos = m*vec4(position,1.0f) + vec4(posBias,0.0f);

	
	gl_Position = projection*view*model*realPos;
}


#shader fragment
#version 460 core 

out vec4 color;

void main() 
{

}