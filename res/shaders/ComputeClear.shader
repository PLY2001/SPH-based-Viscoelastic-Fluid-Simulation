#shader compute
#version 460 core 

layout (local_size_x = 10, local_size_y = 10, local_size_z = 1) in;//�ⲿ����group����,���ﶨ��invocation����
layout (rgba32f, binding = 0) uniform image2D input_image;

//shared vec4 mat_shared[600][600];//���������ͬһ��WorkGroup�е�Invocations������ı�������������һ��Invocationȥ��ʼ��

void main() 
{ 
	//uvec3 gl_NumWorkGroups �ⲿ�����group����
	//uvec3 gl_WorkGroupID ��ǰinvocation������groupID
	//uvec3 gl_LocalInvocationID ��ǰinvocation�ڵ�ǰgroup�еľֲ�ID
	//uvec3 gl_GlobalInvocationID ��ǰinvocation��ȫ��ID, ����gl_WorkGroupID*gl_WorkGroupSize+gl_LocalInvocationID
	//uint gl_LocalInvocationIndex ��ǰinvocation�ڵ�ǰgroup�еľֲ�Index, ����gl_LocalInvocationID.z*gl_WorkGroupSize.x*gl_WorkGroupSize.y  +  gl_LocalInvocationID.y*gl_WorkGroupSize.x  +  gl_LocalInvocationID.x
	
	//mat_shared[pos.x][pos.y]=imageLoad(input_image,pos);
	//barrier();//ȷ������Invocation��ͬ����
    //vec4 data=mat_shared[pos.x][pos.y];

	//ivec2 dim = imageSize(input_image);//��ȡͼƬ��С

	ivec2 pos=ivec2(gl_GlobalInvocationID.xy);
	vec4 data=vec4(0.0f);
    imageStore(input_image,pos.xy,data);
}