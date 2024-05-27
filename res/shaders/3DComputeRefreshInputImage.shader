#shader compute
#version 460 core 

//ע�⣡��������������������pointInputImageר�ã���������������
layout (local_size_x = 5, local_size_y = 5, local_size_z = 5) in;//�ⲿ����group����,���ﶨ��invocation����
layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D output_image;
layout (rgba32f, binding = 2) uniform image2D parameter_image;

//shared vec4 mat_shared[600][600];//���������ͬһ��WorkGroup�е�Invocations������ı�������������һ��Invocationȥ��ʼ��
ivec3 PointDim = ivec3(imageLoad(parameter_image,ivec2(4,0)).xyz);//��ȡ�򼯺�ͼƬ��С
ivec3 PointUVW = ivec3(gl_GlobalInvocationID.xyz);
ivec2 PointPositionUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z);//��ȡ��ǰ�����������
ivec2 PointVelocityUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z + PointDim.z);//��ȡ��ǰ�����������

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

	vec4 positionData=imageLoad(input_image,PointPositionUV);
	vec4 velocityData=imageLoad(input_image,PointVelocityUV);
    imageStore(output_image,PointPositionUV,positionData);
    imageStore(output_image,PointVelocityUV,velocityData);
}