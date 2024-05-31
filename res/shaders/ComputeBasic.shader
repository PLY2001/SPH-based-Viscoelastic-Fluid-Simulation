#shader compute
#version 460 core 

layout (local_size_x = 1, local_size_y = 10, local_size_z = 10) in;//�ⲿ����group����,���ﶨ��invocation����
layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D output_image;
layout (rgba32f, binding = 2) uniform image2D parameter_image;
layout (rgba32f, binding = 3) uniform image2D predict_density_image;
layout (rgba32f, binding = 4) uniform image2D predict_area_input_image;
layout (rgba32f, binding = 5) uniform image2D predict_area_input_start_index_image;
layout (rgba32f, binding = 6) uniform image2D area_output_image;
layout (rgba32f, binding = 7) uniform image2D predict_input_image;

//shared vec4 mat_shared[600][600];//���������ͬһ��WorkGroup�е�Invocations������ı�������������һ��Invocationȥ��ʼ��
float pointRadiusSize = 0.01f;//С��뾶
float collisionDamping = 0.5f;//�߽���ײ˥������
#define PI 3.1415926f
//��ȡ����
ivec3 PointDim = ivec3(imageLoad(parameter_image,ivec2(4,0)).xyz);//��ȡ�򼯺�ͼƬ��С
ivec3 PointUVW = ivec3(gl_GlobalInvocationID.xyz);
ivec2 PointPositionUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z);//��ȡ��ǰ����������� PointUVW.y=0!
ivec2 PointVelocityUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z + PointDim.z);//��ȡ��ǰ�����������
vec4 PointPositionData=imageLoad(input_image,PointPositionUV);//��ȡ��ǰ������
vec4 PointVelocityData=imageLoad(input_image,PointVelocityUV);//��ȡ��ǰ������
vec4 predict_PointPositionData=imageLoad(predict_input_image,PointPositionUV);//��ȡ��ǰԤ��������
vec4 predict_PointVelocityData=imageLoad(predict_input_image,PointVelocityUV);//��ȡ��ǰԤ��������

float DeltaTime = imageLoad(parameter_image,ivec2(0,0)).x;//��ȡ��λʱ��
vec3 SpaceSize = imageLoad(parameter_image,ivec2(0,0)).yzw;//��ȡ�߽�ʵ�ʴ�С
float Gravity = imageLoad(parameter_image,ivec2(2,0)).w;//��ȡ������С
float SmoothingRadius = imageLoad(parameter_image,ivec2(1,0)).w;//��ȡƽ���뾶
float ForceSize = imageLoad(parameter_image,ivec2(3,0)).x;//�ܶ�������С
float TargetDensity = imageLoad(parameter_image,ivec2(2,0)).x;//��ȡĿ���ܶ�
float ViscositySize = imageLoad(parameter_image,ivec2(2,0)).y;//ճ��������С
float NearForceSize = imageLoad(parameter_image,ivec2(2,0)).z;//���ܶ�������С
ivec3 AreaBias[9] = {ivec3(0,-1,-1),ivec3(0,-1,0),ivec3(0,-1,1),ivec3(0,0,-1),ivec3(0,0,0),ivec3(0,0,1),ivec3(0,1,-1),ivec3(0,1,0),ivec3(0,1,1)};
ivec3 AreaCount = ivec3(imageLoad(parameter_image,ivec2(3,0)).yzw);
ivec3 AreaIndexBias = ivec3(imageLoad(parameter_image,ivec2(1,0)).xyz);//��������ƫ����
ivec2 AreaIndexImageDim = ivec2(imageSize(predict_area_input_start_index_image));
float BorderForce = imageLoad(parameter_image,ivec2(4,0)).w;

float SmoothingKernelForViscosity(float dis,float radius){
	if(dis > radius) return 0;
	float value = max(0,radius*radius - dis*dis);
	return value*value*value;
}

float SmoothingKernelDerivative(float dis, float radius){ //���㺯��б��
	if(dis > radius) return 0;
	float scale = 12 / (PI * pow(radius, 4));
	return scale * (dis - radius);// *3.0f*(radius - dis);
}

vec3 GetRandomDir(){ //�������
	float x = fract(sin(DeltaTime)*100000.0*(PointUVW.x+1.0f))*2.0f - 1.0f;
	float y = fract(sin(DeltaTime*2.0f)*1000000.0*(PointUVW.y+2.0f))*2.0f - 1.0f;
	float z = fract(sin(DeltaTime*3.0f)*100000000.0*(PointUVW.z+3.0f))*2.0f - 1.0f;
	return vec3(x,-y,z);
}

vec2 ConvertDensityToPressure(float density, float density_near){ //���ܶ�תΪѹ��
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

vec3 CalculatePressureAndViscosityForce(ivec2 centerPositionIndex,ivec2 centerVelocityIndex){ //����ѹ����ճ�ȵĴ�С�ͷ���
	vec3 finalForce = vec3(0.0f);
	vec3 pressureForce = vec3(0.0f);//ѹ��
	vec3 viscosityForce = vec3(0.0f);//ճ��
	float mass = 1.0f;

	vec3 center_position = vec3(predict_PointPositionData.x,predict_PointPositionData.y,predict_PointPositionData.z);//������������ʵ���е�����
	vec3 center_velocity = vec3(predict_PointVelocityData.x,predict_PointVelocityData.y,predict_PointVelocityData.z);
	float center_density = imageLoad(predict_density_image,centerPositionIndex).x;//��������ܶ�
	float center_density_near = imageLoad(predict_density_image,centerPositionIndex).y;//��������ܶ�

	//��������
	int area_x = int((center_position.x + SpaceSize.x / 2.0f )/ SmoothingRadius + AreaIndexBias.x);//����ʵ������������
	int area_y = int((center_position.y + SpaceSize.y / 2.0f )/ SmoothingRadius + AreaIndexBias.y);//����ʵ������������
	int area_z = int((center_position.z + SpaceSize.z / 2.0f )/ SmoothingRadius + AreaIndexBias.z);//����ʵ������������
	ivec3 center_area_uvw = ivec3(area_x,area_y,area_z);
	int area_i_max = AreaCount.x*AreaCount.y*AreaCount.z - 1;
	ivec2 otherPositionIndex;

	for(int n = 0; n<9; n++){
		ivec3 area_uvw = center_area_uvw + AreaBias[n];
		int area_i = area_uvw.x + area_uvw.y * AreaCount.x + area_uvw.z * AreaCount.x * AreaCount.y;
		if(area_i > -1 && area_i < area_i_max+1) {
			ivec2 area_uv = ivec2(area_i % AreaIndexImageDim.x, area_i / AreaIndexImageDim.x);
			int area_start_index = int(imageLoad(predict_area_input_start_index_image,area_uv).x);//��ȡ��������������ʼ������ 0,0 ~100,100
			int area_point_count = int(imageLoad(predict_area_input_start_index_image,area_uv).z);//��ȡ������������������
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
				vec3 dir = dis==0? GetRandomDir() : offset/dis; //��������������ѹ������
				
				float slope = SmoothingKernelDerivative(dis, SmoothingRadius);//б��
				float density = imageLoad(predict_density_image,otherPositionIndex).x;//��������ܶ�
				float density_near = imageLoad(predict_density_image,otherPositionIndex).y;//��������ܶ�
				
				float sharedPressure = CalculateSharedPressure(density,density_near, center_density,center_density_near);//��������(���������໥)
				pressureForce = pressureForce + sharedPressure * dir * slope * mass / density;//ѹ����С�ͷ���

				float influence = SmoothingKernelForViscosity(dis,SmoothingRadius);
				ivec2 otherVelocityIndex = ivec2(otherPositionIndex.x,otherPositionIndex.y + PointDim.z);
				vec3 other_velocity = imageLoad(predict_input_image,otherVelocityIndex).xyz;
				viscosityForce = viscosityForce + (other_velocity - center_velocity)*influence;//ճ�ȴ�С�ͷ���
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


	//����������������������
	vec3 force = CalculatePressureAndViscosityForce(PointPositionUV, PointVelocityUV);
	
	float center_density = imageLoad(predict_density_image,PointPositionUV).x;//��������ܶ�
	//center_density = center_density==0? TargetDensity : center_density;
	vec3 acceleration = force / center_density;//�������������ļ��ٶȴ�С�ͷ���

	

	

	//����
	velocity = velocity + vec3(0.0,acceleration.y,acceleration.z)*DeltaTime + vec3(0.0f,-1.0f,0.0f)*Gravity*DeltaTime;
	position = position + velocity*DeltaTime;

	//�߽���ײ
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


	int new_area_x = int((position.x + SpaceSize.x / 2.0f )/ SmoothingRadius + AreaIndexBias.x);//����ʵ������������
	int new_area_y = int((position.y + SpaceSize.y / 2.0f )/ SmoothingRadius + AreaIndexBias.y);//����ʵ������������
	int new_area_z = int((position.z + SpaceSize.z / 2.0f )/ SmoothingRadius + AreaIndexBias.z);//����ʵ������������
	int area_i = new_area_x + new_area_y * AreaCount.x + new_area_z * AreaCount.x * AreaCount.y;
	vec4 area_image_data = vec4(area_i,0.0f,PointPositionUV);
	imageStore(area_output_image,PointPositionUV,area_image_data);

	//ivec2 new_uv_in_position_image = ivec2(float(DensityDim.x)*(position.x/densityHalfSize*0.5f+0.5f),float(DensityDim.y)*(position.y/densityHalfSize*0.5f+0.5f));//�������ĵ���λ��ͼ�е�����
	//vec4 position_image_data = vec4(PointUV,1.0f,1.0f);
	//imageStore(position_output_image,new_uv_in_position_image,position_image_data);


}