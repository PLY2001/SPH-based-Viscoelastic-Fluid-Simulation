#shader compute
#version 460 core 

layout (local_size_x = 1, local_size_y = 10, local_size_z = 10) in;//外部定义group数量,这里定义invocation数量
layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D output_image;
layout (rgba32f, binding = 2) uniform image2D parameter_image;
layout (rgba32f, binding = 3) uniform image2D predict_density_image;
layout (rgba32f, binding = 4) uniform image2D predict_area_input_image;
layout (rgba32f, binding = 5) uniform image2D predict_area_input_start_index_image;
layout (rgba32f, binding = 6) uniform image2D area_output_image;
layout (rgba32f, binding = 7) uniform image2D predict_input_image;

//shared vec4 mat_shared[600][600];//共享变量：同一个WorkGroup中的Invocations所共享的变量，它必须由一个Invocation去初始化
float pointRadiusSize = 0.01f;//小球半径
float collisionDamping = 0.5f;//边界碰撞衰减因子
#define PI 3.1415926f
//读取数据
ivec3 PointDim = ivec3(imageLoad(parameter_image,ivec2(4,0)).xyz);//获取球集合图片大小
ivec3 PointUVW = ivec3(gl_GlobalInvocationID.xyz);
ivec2 PointPositionUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z);//获取当前球的纹理坐标 PointUVW.y=0!
ivec2 PointVelocityUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z + PointDim.z);//获取当前球的纹理坐标
vec4 PointPositionData=imageLoad(input_image,PointPositionUV);//获取当前球数据
vec4 PointVelocityData=imageLoad(input_image,PointVelocityUV);//获取当前球数据
vec4 predict_PointPositionData=imageLoad(predict_input_image,PointPositionUV);//获取当前预测球数据
vec4 predict_PointVelocityData=imageLoad(predict_input_image,PointVelocityUV);//获取当前预测球数据

float DeltaTime = imageLoad(parameter_image,ivec2(0,0)).x;//获取单位时间
vec3 SpaceSize = imageLoad(parameter_image,ivec2(0,0)).yzw;//获取边界实际大小
float Gravity = imageLoad(parameter_image,ivec2(2,0)).w;//获取重力大小
float SmoothingRadius = imageLoad(parameter_image,ivec2(1,0)).w;//获取平滑半径
float ForceSize = imageLoad(parameter_image,ivec2(3,0)).x;//密度力量大小
float TargetDensity = imageLoad(parameter_image,ivec2(2,0)).x;//获取目标密度
float ViscositySize = imageLoad(parameter_image,ivec2(2,0)).y;//粘度力量大小
float NearForceSize = imageLoad(parameter_image,ivec2(2,0)).z;//近密度力量大小
ivec3 AreaBias[9] = {ivec3(0,-1,-1),ivec3(0,-1,0),ivec3(0,-1,1),ivec3(0,0,-1),ivec3(0,0,0),ivec3(0,0,1),ivec3(0,1,-1),ivec3(0,1,0),ivec3(0,1,1)};
ivec3 AreaCount = ivec3(imageLoad(parameter_image,ivec2(3,0)).yzw);
ivec3 AreaIndexBias = ivec3(imageLoad(parameter_image,ivec2(1,0)).xyz);//区域索引偏移量
ivec2 AreaIndexImageDim = ivec2(imageSize(predict_area_input_start_index_image));
float BorderForce = imageLoad(parameter_image,ivec2(4,0)).w;

float SmoothingKernelForViscosity(float dis,float radius){
	if(dis > radius) return 0;
	float value = max(0,radius*radius - dis*dis);
	return value*value*value;
}

float SmoothingKernelDerivative(float dis, float radius){ //计算函数斜率
	if(dis > radius) return 0;
	float scale = 12 / (PI * pow(radius, 4));
	return scale * (dis - radius);// *3.0f*(radius - dis);
}

vec3 GetRandomDir(){ //随机方向
	float x = fract(sin(DeltaTime)*100000.0*(PointUVW.x+1.0f))*2.0f - 1.0f;
	float y = fract(sin(DeltaTime*2.0f)*1000000.0*(PointUVW.y+2.0f))*2.0f - 1.0f;
	float z = fract(sin(DeltaTime*3.0f)*100000000.0*(PointUVW.z+3.0f))*2.0f - 1.0f;
	return vec3(x,-y,z);
}

vec2 ConvertDensityToPressure(float density, float density_near){ //将密度转为压力
	float densityError = density - TargetDensity;
	float pressure = densityError * ForceSize;
	float pressure_near = density_near * NearForceSize;
	return vec2(pressure,pressure_near);
}

float CalculateSharedPressure(float density1, float density1_near, float density2, float density2_near){
	vec2 pressure1 = ConvertDensityToPressure(density1,density1_near);
	vec2 pressure2 = ConvertDensityToPressure(density2,density2_near);
	return (pressure1.x + pressure1.y + pressure2.x + pressure2.y)/2.0f;
}

vec3 CalculatePressureAndViscosityForce(ivec2 centerPositionIndex,ivec2 centerVelocityIndex){ //计算压力和粘度的大小和方向
	vec3 finalForce = vec3(0.0f);
	vec3 pressureForce = vec3(0.0f);//压力
	vec3 viscosityForce = vec3(0.0f);//粘度
	float mass = 1.0f;

	vec3 center_position = vec3(predict_PointPositionData.x,predict_PointPositionData.y,predict_PointPositionData.z);//采样中心球在实际中的坐标
	vec3 center_velocity = vec3(predict_PointVelocityData.x,predict_PointVelocityData.y,predict_PointVelocityData.z);
	float center_density = imageLoad(predict_density_image,centerPositionIndex).x;//中心球的密度
	float center_density_near = imageLoad(predict_density_image,centerPositionIndex).y;//中心球的密度

	//查找区域
	int area_x = int((center_position.x + SpaceSize.x / 2.0f )/ SmoothingRadius + AreaIndexBias.x);//根据实际坐标求区域
	int area_y = int((center_position.y + SpaceSize.y / 2.0f )/ SmoothingRadius + AreaIndexBias.y);//根据实际坐标求区域
	int area_z = int((center_position.z + SpaceSize.z / 2.0f )/ SmoothingRadius + AreaIndexBias.z);//根据实际坐标求区域
	ivec3 center_area_uvw = ivec3(area_x,area_y,area_z);
	int area_i_max = AreaCount.x*AreaCount.y*AreaCount.z - 1;
	ivec2 otherPositionIndex;

	for(int n = 0; n<9; n++){
		ivec3 area_uvw = center_area_uvw + AreaBias[n];
		int area_i = area_uvw.x + area_uvw.y * AreaCount.x + area_uvw.z * AreaCount.x * AreaCount.y;
		if(area_i > -1 && area_i < area_i_max+1) {
			ivec2 area_uv = ivec2(area_i % AreaIndexImageDim.x, area_i / AreaIndexImageDim.x);
			int area_start_index = int(imageLoad(predict_area_input_start_index_image,area_uv).x);//获取该区域所含的起始球索引 0,0 ~100,100
			int area_point_count = int(imageLoad(predict_area_input_start_index_image,area_uv).z);//获取该区域所含的球总数
			int area_index = area_start_index;
			for(int i = 0;i<area_point_count;i++){
				int uv_xy_i = area_index % (PointDim.x * PointDim.y);
				int uv_z = area_index / (PointDim.x * PointDim.y);
				ivec2 area_index_uv = ivec2(uv_xy_i,uv_z);
				otherPositionIndex = ivec2(imageLoad(predict_area_input_image,area_index_uv).zw);
				if(otherPositionIndex != centerPositionIndex) {
				vec3 otherPostision = imageLoad(predict_input_image,otherPositionIndex).xyz;

				//otherPostision = otherPostision + GetRandomDir()*pointRadiusSize*0.5f;

				vec3 offset = otherPostision - center_position;
				float dis = distance(otherPostision,center_position);
				vec3 dir = dis==0? GetRandomDir() : offset/dis; //其他球对中心球的压力方向
				
				float slope = SmoothingKernelDerivative(dis, SmoothingRadius);//斜率
				float density = imageLoad(predict_density_image,otherPositionIndex).x;//其他球的密度
				float density_near = imageLoad(predict_density_image,otherPositionIndex).y;//其他球的密度
				
				float sharedPressure = CalculateSharedPressure(density,density_near, center_density,center_density_near);//动量定理(力的作用相互)
				pressureForce = pressureForce + sharedPressure * dir * slope * mass / density;//压力大小和方向

				float influence = SmoothingKernelForViscosity(dis,SmoothingRadius);
				ivec2 otherVelocityIndex = ivec2(otherPositionIndex.x,otherPositionIndex.y + PointDim.z);
				vec3 other_velocity = imageLoad(predict_input_image,otherVelocityIndex).xyz;
				viscosityForce = viscosityForce + (other_velocity - center_velocity)*influence;//粘度大小和方向
				}
				area_index = area_index + 1;

			}
		}
	}

	
	finalForce = pressureForce + viscosityForce * ViscositySize;

	return finalForce;
}


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


	//计算其他球对中心球的作用
	vec3 force = CalculatePressureAndViscosityForce(PointPositionUV, PointVelocityUV);
	
	float center_density = imageLoad(predict_density_image,PointPositionUV).x;//中心球的密度
	//center_density = center_density==0? TargetDensity : center_density;
	vec3 acceleration = force / center_density;//其他球对中心球的加速度大小和方向

	

	

	//更新
	velocity = velocity + vec3(0.0,acceleration.y,acceleration.z)*DeltaTime + vec3(0.0f,-1.0f,0.0f)*Gravity*DeltaTime;
	position = position + velocity*DeltaTime;

	//边界碰撞
	vec3 halfBoundSize = SpaceSize/2.0f - vec3(pointRadiusSize);
	if(abs(position.x) > halfBoundSize.x){
		position.x = halfBoundSize.x * sign(position.x);
		velocity.x = velocity.x * -1 * collisionDamping;//+ sign(position.x) * -1 * BorderForce * DeltaTime;// + BoundZoomAcceleration.x * sign(position.x) * DeltaTime;
	}
	if(abs(position.y) > halfBoundSize.y){
		position.y = halfBoundSize.y * sign(position.y);
		velocity.y = velocity.y * -1 * collisionDamping;// + sign(position.y) * -1 * BorderForce * DeltaTime;// + BoundZoomAcceleration.y * sign(position.y) * DeltaTime;
	}
	if(abs(position.z) > halfBoundSize.z){
		position.z = halfBoundSize.z * sign(position.z);
		velocity.z = velocity.z * -1 * collisionDamping;// + BoundZoomAcceleration.y * sign(position.y) * DeltaTime;
	}

	PointPositionData = vec4(position, 0.0f);
	PointVelocityData = vec4(velocity, 0.0f);
    imageStore(output_image,PointPositionUV,PointPositionData);
    imageStore(output_image,PointVelocityUV,PointVelocityData);


	int new_area_x = int((position.x + SpaceSize.x / 2.0f )/ SmoothingRadius + AreaIndexBias.x);//根据实际坐标求区域
	int new_area_y = int((position.y + SpaceSize.y / 2.0f )/ SmoothingRadius + AreaIndexBias.y);//根据实际坐标求区域
	int new_area_z = int((position.z + SpaceSize.z / 2.0f )/ SmoothingRadius + AreaIndexBias.z);//根据实际坐标求区域
	int area_i = new_area_x + new_area_y * AreaCount.x + new_area_z * AreaCount.x * AreaCount.y;
	vec4 area_image_data = vec4(area_i,0.0f,PointPositionUV);
	imageStore(area_output_image,PointPositionUV,area_image_data);

	//ivec2 new_uv_in_position_image = ivec2(float(DensityDim.x)*(position.x/densityHalfSize*0.5f+0.5f),float(DensityDim.y)*(position.y/densityHalfSize*0.5f+0.5f));//采样中心点在位置图中的坐标
	//vec4 position_image_data = vec4(PointUV,1.0f,1.0f);
	//imageStore(position_output_image,new_uv_in_position_image,position_image_data);


}