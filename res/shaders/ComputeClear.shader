#shader compute
#version 460 core 

layout (local_size_x = 10, local_size_y = 10, local_size_z = 1) in;//外部定义group数量,这里定义invocation数量
layout (rgba32f, binding = 0) uniform image2D input_image;

//shared vec4 mat_shared[600][600];//共享变量：同一个WorkGroup中的Invocations所共享的变量，它必须由一个Invocation去初始化

void main() 
{ 
	//uvec3 gl_NumWorkGroups 外部定义的group数量
	//uvec3 gl_WorkGroupID 当前invocation所属的groupID
	//uvec3 gl_LocalInvocationID 当前invocation在当前group中的局部ID
	//uvec3 gl_GlobalInvocationID 当前invocation的全局ID, 等于gl_WorkGroupID*gl_WorkGroupSize+gl_LocalInvocationID
	//uint gl_LocalInvocationIndex 当前invocation在当前group中的局部Index, 等于gl_LocalInvocationID.z*gl_WorkGroupSize.x*gl_WorkGroupSize.y  +  gl_LocalInvocationID.y*gl_WorkGroupSize.x  +  gl_LocalInvocationID.x
	
	//mat_shared[pos.x][pos.y]=imageLoad(input_image,pos);
	//barrier();//确保所有Invocation是同步的
    //vec4 data=mat_shared[pos.x][pos.y];

	//ivec2 dim = imageSize(input_image);//获取图片大小

	ivec2 pos=ivec2(gl_GlobalInvocationID.xy);
	vec4 data=vec4(0.0f);
    imageStore(input_image,pos.xy,data);
}