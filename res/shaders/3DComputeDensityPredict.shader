#shader compute
#version 460 core 

layout (local_size_x = 5, local_size_y = 5, local_size_z = 5) in;//�ⲿ����group����,���ﶨ��invocation����
layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D density_output_image;
layout (rgba32f, binding = 2) uniform image2D parameter_image;
layout (rgba32f, binding = 3) uniform image2D area_input_image;
layout (rgba32f, binding = 4) uniform image2D area_input_start_index_image;

ivec3 PointDim = ivec3(imageLoad(parameter_image,ivec2(4,0)).xyz);
ivec3 PointUVW = ivec3(gl_GlobalInvocationID.xyz);
ivec2 PointPositionUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z);//��ȡ��ǰ�����������
ivec2 PointVelocityUV = ivec2(PointUVW.x + PointUVW.y*PointDim.x, PointUVW.z + PointDim.z);//��ȡ��ǰ�����������
vec4 PointPositionData=imageLoad(input_image,PointPositionUV);//��ȡ��ǰ������
vec4 PointVelocityData=imageLoad(input_image,PointVelocityUV);//��ȡ��ǰ������
vec3 SpaceSize = imageLoad(parameter_image,ivec2(0,0)).yzw;//��ȡ�߽�ʵ�ʴ�С
float SmoothingRadius = imageLoad(parameter_image,ivec2(1,0)).w;//��ȡƽ���뾶
ivec3 AreaBias[27] = {ivec3(-1,-1,-1),ivec3(0,-1,-1),ivec3(1,-1,-1),ivec3(-1,0,-1),ivec3(0,0,-1),ivec3(1,0,-1),ivec3(-1,1,-1),ivec3(0,1,-1),ivec3(1,1,-1),ivec3(-1,-1,0),ivec3(0,-1,0),ivec3(1,-1,0),ivec3(-1,0,0),ivec3(0,0,0),ivec3(1,0,0),ivec3(-1,1,0),ivec3(0,1,0),ivec3(1,1,0),ivec3(-1,-1,1),ivec3(0,-1,1),ivec3(1,-1,1),ivec3(-1,0,1),ivec3(0,0,1),ivec3(1,0,1),ivec3(-1,1,1),ivec3(0,1,1),ivec3(1,1,1)};
ivec3 AreaIndexBias = ivec3(imageLoad(parameter_image,ivec2(1,0)).xyz);//��������ƫ����
ivec3 AreaCount = ivec3(imageLoad(parameter_image,ivec2(3,0)).yzw);
ivec2 AreaIndexImageDim = ivec2(imageSize(area_input_start_index_image));

#define PI 3.1415926f

float SmoothingKernel(float radius, float dis){
	if(dis>radius) return 0;
	float volume = (PI * pow(radius,4.0f)) / 6.0f;
	return (radius - dis) * (radius - dis) / volume;
}

float SmoothingNearKernel(float radius, float dis){
	if(dis>radius) return 0;
	float volume = (PI * pow(radius,5.0f)) / 10.0f;//(r - x)^3��y����ת360�ȵĻ������
	return (radius - dis) * (radius - dis) *(radius - dis)/ volume;
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

	//ivec2 dim = imageSize(input_image);//��ȡͼƬ��С

	const float mass = 1;
	float density = 0.0f;
	float density_near = 0.0f;

	vec3 center_position = vec3(PointPositionData.x,PointPositionData.y,PointPositionData.z);//������������ʵ���е�����
	
	//��������
	int area_x = int((center_position.x + SpaceSize.x / 2.0f )/ SmoothingRadius + AreaIndexBias.x);//����ʵ������������
	int area_y = int((center_position.y + SpaceSize.y / 2.0f )/ SmoothingRadius + AreaIndexBias.y);//����ʵ������������
	int area_z = int((center_position.z + SpaceSize.z / 2.0f )/ SmoothingRadius + AreaIndexBias.z);//����ʵ������������
	ivec3 center_area_uvw = ivec3(area_x,area_y,area_z);
	int area_i_max = AreaCount.x*AreaCount.y*AreaCount.z - 1;
	ivec2 otherIndex;

	for(int n = 0; n<27; n++){
		ivec3 area_uvw = center_area_uvw + AreaBias[n];
		int area_i = area_uvw.x + area_uvw.y * AreaCount.x + area_uvw.z * AreaCount.x * AreaCount.y;
		if(area_i > -1 && area_i < area_i_max+1) {
			ivec2 area_uv = ivec2(area_i % AreaIndexImageDim.x, area_i / AreaIndexImageDim.x);
			int area_start_index = int(imageLoad(area_input_start_index_image,area_uv).x);//��ȡ��������������ʼ������
			int area_point_count = int(imageLoad(area_input_start_index_image,area_uv).z);//��ȡ������������������
			int area_index = area_start_index;
			for(int i = 0;i<area_point_count;i++){
				int uv_xy_i = area_index % (PointDim.x * PointDim.y);
				int uv_z = area_index / (PointDim.x * PointDim.y);
				ivec2 area_index_uv = ivec2(uv_xy_i,uv_z);
				otherIndex = ivec2(imageLoad(area_input_image,area_index_uv).zw);
				vec3 other_position = imageLoad(input_image,otherIndex).xyz;
				float dis = distance(other_position,center_position);
				float influence = SmoothingKernel(SmoothingRadius,dis);//������Ը��ܶ�ͼ��������ܶ�
				float influence_near = SmoothingNearKernel(SmoothingRadius,dis);//������Ը��ܶ�ͼ��������ܶ�
				density = density + influence*mass;
				density_near = density_near + influence_near*mass;
				area_index = area_index + 1;
			}
		}
	}


	//����ȫ����
	//for(int i = -DensitySmoothingRadiusUV;i<=DensitySmoothingRadiusUV;i++){
	//	for(int j = -DensitySmoothingRadiusUV;j<=DensitySmoothingRadiusUV;j++){
	//		//��ǰ���������ܶ�ͼ�е�����//0~20
	//		if((DensityUV.x + i)>(DensityDim.x - 1) || (DensityUV.x + i)<0 || (DensityUV.y + j)>(DensityDim.y - 1) || (DensityUV.y + j)<0){
	//			continue;
	//		}
	//		else{
	//			ivec2 this_uv = ivec2(DensityUV.x + i, DensityUV.y + j);
	//			if(imageLoad(position_input_image,this_uv).z < 0.5f){
	//				continue;
	//			}
	//			else{
	//				ivec2 otherIndex = ivec2(imageLoad(position_input_image,this_uv).xy);
	//				vec2 position = imageLoad(input_image,otherIndex).xy;
	//				float dis = distance(position,DensityPosition);
	//				float influence = SmoothingKernel(SmoothingRadius,dis);//������Ը��ܶ�ͼ��������ܶ�
	//				float influence_near = SmoothingNearKernel(SmoothingRadius,dis);//������Ը��ܶ�ͼ��������ܶ�
	//				density = density + influence*mass;
	//				density_near = density_near + influence_near*mass;
	//
	//			}
	//		}
	//
	//	}
	//}

	vec4 result = vec4(density,density_near,0.0f,0.0f);
	imageStore(density_output_image,PointPositionUV,result);


	
    
}