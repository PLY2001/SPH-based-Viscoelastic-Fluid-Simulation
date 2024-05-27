#shader compute
#version 460 core 

//注意！！！！！！！！！！！pointInputImage专用！！！！！！！！
layout (local_size_x = 5, local_size_y = 5, local_size_z = 5) in;//外部定义group数量,这里定义invocation数量
layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D output_image;
layout (rgba32f, binding = 2) uniform image2D parameter_image;

//shared vec4 mat_shared[600][600];//共享变量：同一个WorkGroup中的Invocations所共享的变量，它必须由一个Invocation去初始化
ivec3 PointDim = ivec3(imageLoad(parameter_image,ivec2(4,0)).xyz);//获取球集合图片大小
ivec3 PointUVW = ivec3(gl_GlobalInvocationID.xyz);
ivec2 PointPositionUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z);//获取当前球的纹理坐标
ivec2 PointVelocityUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z + PointDim.z);//获取当前球的纹理坐标

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

	vec4 positionData=imageLoad(input_image,PointPositionUV);
	vec4 velocityData=imageLoad(input_image,PointVelocityUV);
    imageStore(output_image,PointPositionUV,positionData);
    imageStore(output_image,PointVelocityUV,velocityData);
}