#shader compute
#version 460 core 

layout (local_size_x = 5, local_size_y = 5, local_size_z = 5) in;//�ⲿ����group����,���ﶨ��invocation����
layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D output_image;
layout (rgba32f, binding = 2) uniform image2D parameter_image;
layout (rgba32f, binding = 3) uniform image2D area_output_image;

float pointRadiusSize = 0.01f;//С��뾶
float collisionDamping = 0.5f;//�߽���ײ˥������
#define PI 3.1415926f
//��ȡ����
ivec3 PointDim = ivec3(imageLoad(parameter_image,ivec2(4,0)).xyz);
ivec3 PointUVW = ivec3(gl_GlobalInvocationID.xyz);
ivec2 PointPositionUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z);//��ȡ��ǰ�����������
ivec2 PointVelocityUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z + PointDim.z);//��ȡ��ǰ�����������
vec4 PointPositionData=imageLoad(input_image,PointPositionUV);//��ȡ��ǰ������
vec4 PointVelocityData=imageLoad(input_image,PointVelocityUV);//��ȡ��ǰ������
float DeltaTime = imageLoad(parameter_image,ivec2(0,0)).x;//��ȡ��λʱ��
vec3 SpaceSize = imageLoad(parameter_image,ivec2(0,0)).yzw;//��ȡ�߽�ʵ�ʴ�С
float SmoothingRadius = imageLoad(parameter_image,ivec2(1,0)).w;//��ȡƽ���뾶
ivec3 AreaBias = ivec3(imageLoad(parameter_image,ivec2(1,0)).xyz);//��������ƫ����
ivec3 AreaCount = ivec3(imageLoad(parameter_image,ivec2(3,0)).yzw);





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

	//����������
	vec3 position = vec3(PointPositionData.x,PointPositionData.y,PointPositionData.z);
	vec3 velocity = vec3(PointVelocityData.x,PointVelocityData.y,PointVelocityData.z);

	//����
	position = position + velocity*1.0f/120.0f;

	//�߽���ײ
	vec3 halfBoundSize = SpaceSize/2.0f - vec3(pointRadiusSize);
	if(abs(position.x) > halfBoundSize.x){
		position.x = halfBoundSize.x * sign(position.x);
		velocity.x = velocity.x * -1 * collisionDamping;// + BoundZoomAcceleration.x * sign(position.x) * DeltaTime;
	}
	if(abs(position.y) > halfBoundSize.y){
		position.y = halfBoundSize.y * sign(position.y);
		velocity.y = velocity.y * -1 * collisionDamping;// + BoundZoomAcceleration.y * sign(position.y) * DeltaTime;
	}
	if(abs(position.z) > halfBoundSize.z){
		position.z = halfBoundSize.z * sign(position.z);
		velocity.z = velocity.z * -1 * collisionDamping;// + BoundZoomAcceleration.y * sign(position.y) * DeltaTime;
	}

	PointPositionData = vec4(position, 0.0f);
	PointVelocityData = vec4(velocity, 0.0f);
    imageStore(output_image,PointPositionUV,PointPositionData);
    imageStore(output_image,PointVelocityUV,PointVelocityData);

	int area_x = int((position.x + SpaceSize.x / 2.0f )/ SmoothingRadius + AreaBias.x);//����ʵ������������
	int area_y = int((position.y + SpaceSize.y / 2.0f )/ SmoothingRadius + AreaBias.y);//����ʵ������������
	int area_z = int((position.z + SpaceSize.z / 2.0f )/ SmoothingRadius + AreaBias.z);//����ʵ������������
	int area_i = area_x + area_y * AreaCount.x + area_z * AreaCount.x * AreaCount.y;
	vec4 area_image_data = vec4(area_i,0.0f,PointPositionUV);
	imageStore(area_output_image,PointPositionUV,area_image_data);

	//float positionHalfSize = PositionSize/2.0f;//0.25
	//ivec2 new_uv_in_position_image = ivec2(float(PositionDim.x)*(position.x/positionHalfSize*0.5f+0.5f),float(PositionDim.y)*(position.y/positionHalfSize*0.5f+0.5f));//�������ĵ���λ��ͼ�е�����
	//vec4 position_image_data = vec4(PointUV,1.0f,1.0f);
	//imageStore(position_output_image,new_uv_in_position_image,position_image_data);


}