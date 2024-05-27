#shader compute
#version 460 core 

layout (local_size_x = 10, local_size_y = 10, local_size_z = 1) in;//�ⲿ����group����,���ﶨ��invocation����
layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D output_image;
layout (rgba32f, binding = 2) uniform image2D parameter_image;
layout (rgba32f, binding = 3) uniform image2D area_input_image;
layout (rgba32f, binding = 4) uniform image2D area_input_start_index_image;

ivec2 PointDim = imageSize(input_image);//��ȡ�򼯺�ͼƬ��С
ivec2 PointUV=ivec2(gl_GlobalInvocationID.xy);//��ȡ��ǰ�����������
vec4 PointData=imageLoad(input_image,PointUV);//��ȡ��ǰ������

ivec2 DensityUV=ivec2(gl_GlobalInvocationID.xy);//��ȡ��ǰ�ܶȵ���������
ivec2 DensityDim = imageSize(output_image);//��ȡ�ܶ�ͼͼƬ��С//20
vec2 SpaceSize = imageLoad(parameter_image,ivec2(0,0)).yz;//��ȡ�߽�ʵ�ʴ�С
float DensitySize = max(SpaceSize.x,SpaceSize.y);//��ȡ�ܶ�ʵ�ʴ�С//0.5
float SmoothingRadius = imageLoad(parameter_image,ivec2(1,0)).z;//��ȡƽ���뾶
float UVStepSize = DensitySize / max(DensityDim.x,DensityDim.y);//�ܶ�ͼһ���ʵ�ʾ���
float HalfDensitySize = DensitySize/2.0f;
vec2 DensityPosition = vec2(-HalfDensitySize + UVStepSize/2.0f + DensityUV.x * UVStepSize, -HalfDensitySize + + UVStepSize/2.0f + DensityUV.y * UVStepSize);
int DensitySmoothingRadiusUV = int(SmoothingRadius/DensitySize*max(DensityDim.x,DensityDim.y));//���ܶ�ͼ������ƽ���뾶��С//4
ivec2 AreaBias[9] = {ivec2(-1,-1),ivec2(0,-1),ivec2(1,-1),ivec2(-1,0),ivec2(0,0),ivec2(1,0),ivec2(-1,1),ivec2(0,1),ivec2(1,1)};		
ivec2 AreaCountDim = imageSize(area_input_start_index_image);

#define PI 3.1415926f

float SmoothingKernel(float radius, float dis){
	if(dis>radius) return 0;
	float volume = (PI * pow(radius,4.0f)) / 6.0f;//(r - x)^2��y����ת360�ȵĻ������
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

	vec2 center_position = vec2(PointData.x,PointData.y);//������������ʵ���е�����

	//��������
	int area_x = int((center_position.x + SpaceSize.x / 2.0f )/ SmoothingRadius);//����ʵ������������
	int area_y = int((center_position.y + SpaceSize.y / 2.0f )/ SmoothingRadius);//����ʵ������������
	ivec2 center_area_uv = ivec2(area_x,area_y);
	for(int n = 0; n<9; n++){
		ivec2 area_uv = center_area_uv + AreaBias[n];
		if(area_uv.x < AreaCountDim.x && area_uv.x > -1 && area_uv.y < AreaCountDim.y && area_uv.y > -1) {
			ivec2 area_start_index = ivec2(imageLoad(area_input_start_index_image,area_uv).xy);//��ȡ��������������ʼ������
			int area_point_count = int(imageLoad(area_input_start_index_image,area_uv).z);//��ȡ������������������

			ivec2 area_input_image_dim = imageSize(area_input_image);//��ȡ�����򼯺�ͼƬ��С

			ivec2 area_index = area_start_index;
			ivec2 otherIndex;
			for(int i = 0;i<area_point_count;i++){
				otherIndex = ivec2(imageLoad(area_input_image,area_index).zw);
				vec2 other_position = imageLoad(input_image,otherIndex).xy;
				float dis = distance(other_position,center_position);
				float influence = SmoothingKernel(SmoothingRadius,dis);//������Ը��ܶ�ͼ��������ܶ�
				float influence_near = SmoothingNearKernel(SmoothingRadius,dis);//������Ը��ܶ�ͼ��������ܶ�
				density = density + influence*mass;
				density_near = density_near + influence_near*mass;

				area_index.x = area_index.x + 1;
				if(area_index.x > (area_input_image_dim.x - 1)){
					area_index.x = 0;
					area_index.y = area_index.y + 1;
				}
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
	//			}
	//		}
	//
	//	}
	//}

	vec4 result = vec4(density,density_near,0.0f,0.0f);
	imageStore(output_image,PointUV,result);


	
    
}