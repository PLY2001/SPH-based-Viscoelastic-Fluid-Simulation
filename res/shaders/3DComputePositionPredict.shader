#shader compute
#version 460 core 

layout (local_size_x = 5, local_size_y = 5, local_size_z = 5) in;//外部定义group数量,这里定义invocation数量
layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D output_image;
layout (rgba32f, binding = 2) uniform image2D parameter_image;
layout (rgba32f, binding = 3) uniform image2D area_output_image;

float pointRadiusSize = 0.01f;//小球半径
float collisionDamping = 0.5f;//边界碰撞衰减因子
#define PI 3.1415926f
//读取数据
ivec3 PointDim = ivec3(imageLoad(parameter_image,ivec2(4,0)).xyz);
ivec3 PointUVW = ivec3(gl_GlobalInvocationID.xyz);
ivec2 PointPositionUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z);//获取当前球的纹理坐标
ivec2 PointVelocityUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z + PointDim.z);//获取当前球的纹理坐标
vec4 PointPositionData=imageLoad(input_image,PointPositionUV);//获取当前球数据
vec4 PointVelocityData=imageLoad(input_image,PointVelocityUV);//获取当前球数据
float DeltaTime = imageLoad(parameter_image,ivec2(0,0)).x;//获取单位时间
vec3 SpaceSize = imageLoad(parameter_image,ivec2(0,0)).yzw;//获取边界实际大小
float SmoothingRadius = imageLoad(parameter_image,ivec2(1,0)).w;//获取平滑半径
ivec3 AreaBias = ivec3(imageLoad(parameter_image,ivec2(1,0)).xyz);//区域索引偏移量
ivec3 AreaCount = ivec3(imageLoad(parameter_image,ivec2(3,0)).yzw);





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

	//中心球数据
	vec3 position = vec3(PointPositionData.x,PointPositionData.y,PointPositionData.z);
	vec3 velocity = vec3(PointVelocityData.x,PointVelocityData.y,PointVelocityData.z);

	//更新
	position = position + velocity*1.0f/120.0f;

	//边界碰撞
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

	int area_x = int((position.x + SpaceSize.x / 2.0f )/ SmoothingRadius + AreaBias.x);//根据实际坐标求区域
	int area_y = int((position.y + SpaceSize.y / 2.0f )/ SmoothingRadius + AreaBias.y);//根据实际坐标求区域
	int area_z = int((position.z + SpaceSize.z / 2.0f )/ SmoothingRadius + AreaBias.z);//根据实际坐标求区域
	int area_i = area_x + area_y * AreaCount.x + area_z * AreaCount.x * AreaCount.y;
	vec4 area_image_data = vec4(area_i,0.0f,PointPositionUV);
	imageStore(area_output_image,PointPositionUV,area_image_data);

	//float positionHalfSize = PositionSize/2.0f;//0.25
	//ivec2 new_uv_in_position_image = ivec2(float(PositionDim.x)*(position.x/positionHalfSize*0.5f+0.5f),float(PositionDim.y)*(position.y/positionHalfSize*0.5f+0.5f));//采样中心点在位置图中的坐标
	//vec4 position_image_data = vec4(PointUV,1.0f,1.0f);
	//imageStore(position_output_image,new_uv_in_position_image,position_image_data);


}