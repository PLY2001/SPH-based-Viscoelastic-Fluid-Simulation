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
layout (rgba32f, binding = 0) uniform image2D density_image;
uniform float DensityDisplayWidth;
uniform float DensityDisplayHeight;
uniform float MaxDensity;
uniform float TargetDensity;

void main() 
{
	vec2 uv = vec2(gl_FragCoord.x/DensityDisplayWidth, gl_FragCoord.y/DensityDisplayHeight);

	ivec2 dim = imageSize(density_image);//获取图片大小
	float density = imageLoad(density_image,ivec2(uv.x*dim.x,uv.y*dim.y)).x;
	//float targetError = (TargetDensity - density) / 400.0f;

	float cut1 = TargetDensity * 2.0f/3.0f;
	float cut2 = TargetDensity * 2.0f/3.0f*2.0f;
	float R = -1.0f + density/cut1;
	float G = density>400.0f*0.5f? 3.0f - density*2.0f/cut2 : density/cut1;
	float B = 2.0f - density*2.0f/cut2;

	//vec3 finalColor = vec3(1.0f-targetError,1.0f-abs(targetError),1.0f+targetError);
	vec3 finalColor = vec3(R,G,B);

	//if(density <= 10) finalColor = vec3(0.0f,0.0f,0.0f);
	//vec3 finalColor = vec3(0.0f,0.0f,density/400.0f);
	color = vec4(finalColor,1.0f);
	//float gamma=2.2;
    //color =vec4(pow(texColor,vec3(1.0/gamma)),1.0f);
}