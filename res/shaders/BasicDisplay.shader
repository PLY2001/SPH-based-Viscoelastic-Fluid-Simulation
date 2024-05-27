#shader vertex
#version 460 core 

layout(location=0) in vec3 position; 
layout(location=1) in vec2 aTexcoords;

out vec2 Texcoords;

void main() 
{ 
	gl_Position = vec4(position,1.0f);
	Texcoords=aTexcoords;
}


#shader fragment
#version 460 core 

in vec2 Texcoords;

out vec4 color; 
uniform sampler2D screenTexture;
layout (rgba32f, binding = 0) uniform image2D image;
uniform float DisplayWidth;
uniform float DisplayHeight;
uniform float MaxSize;

void main() 
{
	vec2 uv = vec2(gl_FragCoord.x/DisplayWidth, gl_FragCoord.y/DisplayHeight);

	ivec2 dim = imageSize(image);//获取图片大小
	float size = imageLoad(image,ivec2(uv.x*dim.x,uv.y*dim.y)).z > 0.5f? 1.0f:0.0f;
	vec2 gg = size>0? imageLoad(image,ivec2(uv.x*dim.x,uv.y*dim.y)).xy/25.0f*0.5f + 0.5f : vec2(0.0f);
	vec3 finalColor = vec3(gg.x,0.0f,gg.y);
	color = vec4(finalColor,1.0f);
}