#shader compute
#version 460 core 

layout (local_size_x = 10, local_size_y = 10, local_size_z = 1) in;//外部定义group数量,这里定义invocation数量
layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D output_image;
layout (rgba32f, binding = 2) uniform image2D parameter_image;
layout (rgba32f, binding = 3) uniform image2D area_input_image;
layout (rgba32f, binding = 4) uniform image2D area_input_start_index_image;

ivec2 PointDim = imageSize(input_image);//获取球集合图片大小
ivec2 PointUV=ivec2(gl_GlobalInvocationID.xy);//获取当前球的纹理坐标
vec4 PointData=imageLoad(input_image,PointUV);//获取当前球数据

ivec2 DensityUV=ivec2(gl_GlobalInvocationID.xy);//获取当前密度的纹理坐标
ivec2 DensityDim = imageSize(output_image);//获取密度图图片大小//20
vec2 SpaceSize = imageLoad(parameter_image,ivec2(0,0)).yz;//获取边界实际大小
float DensitySize = max(SpaceSize.x,SpaceSize.y);//获取密度实际大小//0.5
float SmoothingRadius = imageLoad(parameter_image,ivec2(1,0)).z;//获取平滑半径
float UVStepSize = DensitySize / max(DensityDim.x,DensityDim.y);//密度图一格的实际距离
float HalfDensitySize = DensitySize/2.0f;
vec2 DensityPosition = vec2(-HalfDensitySize + UVStepSize/2.0f + DensityUV.x * UVStepSize, -HalfDensitySize + + UVStepSize/2.0f + DensityUV.y * UVStepSize);
int DensitySmoothingRadiusUV = int(SmoothingRadius/DensitySize*max(DensityDim.x,DensityDim.y));//在密度图采样的平滑半径大小//4
ivec2 AreaBias[9] = {ivec2(-1,-1),ivec2(0,-1),ivec2(1,-1),ivec2(-1,0),ivec2(0,0),ivec2(1,0),ivec2(-1,1),ivec2(0,1),ivec2(1,1)};		
ivec2 AreaCountDim = imageSize(area_input_start_index_image);

#define PI 3.1415926f

float SmoothingKernel(float radius, float dis){
	if(dis>radius) return 0;
	float volume = (PI * pow(radius,4.0f)) / 6.0f;//(r - x)^2绕y轴旋转360度的积分体积
	return (radius - dis) * (radius - dis) / volume;
}

float SmoothingNearKernel(float radius, float dis){
	if(dis>radius) return 0;
	float volume = (PI * pow(radius,5.0f)) / 10.0f;//(r - x)^3绕y轴旋转360度的积分体积
	return (radius - dis) * (radius - dis) *(radius - dis)/ volume;
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

	//ivec2 dim = imageSize(input_image);//获取图片大小

	const float mass = 1;
	float density = 0.0f;
	float density_near = 0.0f;

	vec2 center_position = vec2(PointData.x,PointData.y);//采样中心球在实际中的坐标

	//查找区域
	int area_x = int((center_position.x + SpaceSize.x / 2.0f )/ SmoothingRadius);//根据实际坐标求区域
	int area_y = int((center_position.y + SpaceSize.y / 2.0f )/ SmoothingRadius);//根据实际坐标求区域
	ivec2 center_area_uv = ivec2(area_x,area_y);
	for(int n = 0; n<9; n++){
		ivec2 area_uv = center_area_uv + AreaBias[n];
		if(area_uv.x < AreaCountDim.x && area_uv.x > -1 && area_uv.y < AreaCountDim.y && area_uv.y > -1) {
			ivec2 area_start_index = ivec2(imageLoad(area_input_start_index_image,area_uv).xy);//获取该区域所含的起始球索引
			int area_point_count = int(imageLoad(area_input_start_index_image,area_uv).z);//获取该区域所含的球总数

			ivec2 area_input_image_dim = imageSize(area_input_image);//获取球区域集合图片大小

			ivec2 area_index = area_start_index;
			ivec2 otherIndex;
			for(int i = 0;i<area_point_count;i++){
				otherIndex = ivec2(imageLoad(area_input_image,area_index).zw);
				vec2 other_position = imageLoad(input_image,otherIndex).xy;
				float dis = distance(other_position,center_position);
				float influence = SmoothingKernel(SmoothingRadius,dis);//计算球对该密度图采样点的密度
				float influence_near = SmoothingNearKernel(SmoothingRadius,dis);//计算球对该密度图采样点的密度
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
	

	//遍历全部球
	//for(int i = -DensitySmoothingRadiusUV;i<=DensitySmoothingRadiusUV;i++){
	//	for(int j = -DensitySmoothingRadiusUV;j<=DensitySmoothingRadiusUV;j++){
	//		//当前采样点在密度图中的坐标//0~20
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
	//				float influence = SmoothingKernel(SmoothingRadius,dis);//计算球对该密度图采样点的密度
	//				float influence_near = SmoothingNearKernel(SmoothingRadius,dis);//计算球对该密度图采样点的密度
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