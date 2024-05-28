// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
#include <GL/glew.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <windows.h>

#include "Shader.h"
#include "Renderer.h"
#include "Model.h"
#include "UniformBuffer.h"
#include "InstanceBuffer.h"
#include "FrameBuffer.h"
#include "Texture.h"
#include "Camera.h"

#include <stack> //栈

#include "Config.h"

#include "Application.h"

#include <math.h>

#include <algorithm>

MyApp::MyApplication& App = MyApp::MyApplication::GetInstance();//获取唯一的实例引用

std::vector<glm::mat4> modelMatrixList;
glm::mat4 planeModelMatrix;

//摄像机控制
Camera camera;
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

enum class MouseState {
	OutOfDisplay, InDisplay
};
MouseState mouseState = MouseState::OutOfDisplay;//鼠标所属区域

enum class MouseRole {
	UI, ControlCamera
};
MouseRole mouseRole = MouseRole::UI;//鼠标作用

enum class PointMode {
	xy, xyz
};
PointMode pointMode = PointMode::xy;//2D或3D流体

enum class VisualMode {
	Density, Velocity
};
VisualMode visualMode = VisualMode::Density;//可视化类型

#define PI 3.1415926f

float deltaTime = 0;//每次循环耗时
float lastTime = 0;//上一次记录时间

glm::vec3 DirectLightPos = glm::vec3(40.0f, 0.0f, 0.0f);

//窗口尺寸
unsigned int WinWidth = 1440;// 1330;
unsigned int WinHeight = 900;// 670;
//渲染显示尺寸
unsigned int DisplayWidth = 1030;
unsigned int DisplayHeight = 793;
//密度图渲染显示尺寸
//unsigned int DensityDisplayWidth = 300;
//unsigned int DensityDisplayHeight = 300;
//shadow map
unsigned int ShadowMapWidth = 2048;
unsigned int ShadowMapHeight = 2048;
//compute shader 贴图尺寸
#define CSImageWidth 30
#define CSImageHeight 30
#define CSImageDepth 1
#define CSImageWidth3D 20
#define CSImageHeight3D 20
#define CSImageDepth3D 20
//compute shader 组和线程大小
#define Group_a 3
#define Group_b 3
#define Group_c 1
#define Group_a_3D 4
#define Group_b_3D 4
#define Group_c_3D 4
#define Invocation_a 10
#define Invocation_b 10
#define Invocation_c 1
#define Invocation_a_3D 5
#define Invocation_b_3D 5
#define Invocation_c_3D 5
//compute shader 流体区域
#define SPACE_MIN_SCALE 0.1f
#define SPACE_MAX_SCALE 10.0f
#define SPACE_X 1.5f
#define SPACE_Y 1.5f
#define SPACE_Z 0.05f
#define SPACE_X_3D 2.0f
#define SPACE_Y_3D 2.0f
#define SPACE_Z_3D 2.0f
float CSSpaceWidth = SPACE_X;
float CSSpaceHeight = SPACE_Y;
float CSSpaceDepth = SPACE_Z;
// float CSSpaceWidthVelocity = 0.0f;
// float CSSpaceHeightVelocity = 0.0f;

// float CSSpaceLastWidth = CSSpaceWidth;
// float CSSpaceLastHeight = CSSpaceHeight;
// float CSSpaceLastWidthVelocity = 0.0f;
// float CSSpaceLastHeightVelocity = 0.0f;
//compute shader 参数数量
#define CSParameterWidth 5
#define CSParameterHeight 1
//compute shader 重力
float gravity = 0.0f;
#define GRAVITY_MIN_SCALE -9.8f
#define GRAVITY_MAX_SCALE 9.8f
//compute shader 密度图尺寸
//#define CSDensityImageScale 2
//#define CSDensityImageWidth (CSImageWidth*CSDensityImageScale)
//#define CSDensityImageHeight (CSImageHeight*CSDensityImageScale)
//compute shader 密度图组和线程大小
//#define Group_Density_a Group_a*CSDensityImageScale
//#define Group_Density_b Group_b*CSDensityImageScale
//#define Group_Density_c 1
//#define Invocation_Density_a Invocation_a
//#define Invocation_Density_b Invocation_b
//#define Invocation_Density_c 1
//密度采样范围
#define SmoothingRadiusDefault 0.1f
#define SmoothingRadiusDefault3D 0.25f
float SmoothingRadiusMin = 0.1f;
float SmoothingRadiusMax = (CSSpaceWidth / 2.0f);
float SmoothingRadius = glm::min(SmoothingRadiusDefault, SmoothingRadiusMax);
//密度力量大小, 力量过大会导致液体表面有飞球
#define FORCE_MIN_SCALE 0.0f
#define FORCE_SET_SCALE 0.0f
#define FORCE_MAX_SCALE 1000.0f
float forceSize = FORCE_SET_SCALE;
//float TotalTime = 20.0f;
//目标密度
float targetDensity = 250.0f;
#define DENSITY_MIN_SCALE 1.0f
#define DENSITY_MAX_SCALE 5000.0f
//粘度力量大小
#define VISCOSITY_MIN_SCALE 0.0f
#define VISCOSITY_MAX_SCALE 10000000000.0f
#define VISCOSITY_SCALE 1000.0f
float viscositySize = 0.0f;
//近密度力量大小
//#define FORCENEAR_MIN_SCALE FORCE_MIN_SCALE
//#define FORCENEAR_SET_SCALE FORCE_SET_SCALE
//#define FORCENEAR_MAX_SCALE FORCE_MAX_SCALE
float forceNearSize = FORCE_SET_SCALE/10.0f;
//区域个数
int areaCount_x = ceil(SPACE_MAX_SCALE / SmoothingRadiusMin);
int areaCount_y = ceil(SPACE_MAX_SCALE / SmoothingRadiusMin);
int areaCount_z = 1;
int areaCount_x_3D = ceil(SPACE_MAX_SCALE / SmoothingRadiusMin);
int areaCount_y_3D = ceil(SPACE_MAX_SCALE / SmoothingRadiusMin);
int areaCount_z_3D = ceil(SPACE_MAX_SCALE / SmoothingRadiusMin);
int areaCountImageWidth = areaCount_x;
int areaCountImageWidth3D = ceil(sqrt(areaCount_x_3D * areaCount_y_3D * areaCount_z_3D));
int areaCountImageHeight = areaCount_y;
int areaCountImageHeight3D = areaCountImageWidth3D;
int areaCertainCount_x = ceil(CSSpaceWidth / SmoothingRadius);
int areaCertainCount_y = ceil(CSSpaceHeight / SmoothingRadius);
int areaCertainCount_z = 1;
int areaCertainCount_x_3D = ceil(CSSpaceWidth / SmoothingRadius);
int areaCertainCount_y_3D = ceil(CSSpaceHeight / SmoothingRadius);
int areaCertainCount_z_3D = ceil(CSSpaceDepth / SmoothingRadius);
int areaBias_x = (areaCount_x - areaCertainCount_x) / 2.0f;
int areaBias_y = (areaCount_y - areaCertainCount_y) / 2.0f;
int areaBias_z = (areaCount_z - areaCertainCount_z) / 2.0f;
int areaBias_x_3D = (areaCount_x_3D - areaCertainCount_x_3D) / 2.0f;
int areaBias_y_3D = (areaCount_y_3D - areaCertainCount_y_3D) / 2.0f;
int areaBias_z_3D = (areaCount_z_3D - areaCertainCount_z_3D) / 2.0f;
//可视化最大速度
float MaxVelocity = 10.0f;
#define VELOCITY_MIN_SCALE 1.0f
#define VELOCITY_MAX_SCALE 100.0f
//边界排斥力
float BorderForce = 5.0f;

struct Point {
	float i;
	float gg;
	float u;
	float v;
};

bool cmp(Point a, Point b) {
	return a.i < b.i;
}



void SortAreaOutputImage(PointMode mode, Texture& areaOutputImage, Texture& areaOutputStartIndexImage) {
	Point* data = (Point*)areaOutputImage.GetDataFromTextureFloat(4);

	int ImageWidth;
	int ImageHeight;
	int ImageDepth;
	int aCountImageWidth;
	int aCountImageHeight;
	int aCountImageDepth;
	int aCertainCount_x;
	int aCertainCount_y;
	int aCertainCount_z;

	switch (mode)
	{
	case PointMode::xy:
		ImageWidth = CSImageWidth;
		ImageHeight = CSImageHeight;
		ImageDepth = CSImageDepth;
		aCountImageWidth = areaCountImageWidth;
		aCountImageHeight = areaCountImageHeight;
		aCertainCount_x = areaCertainCount_x;
		aCertainCount_y = areaCertainCount_y;
		aCertainCount_z = areaCertainCount_z;
		break;
	case PointMode::xyz:
		ImageWidth = CSImageWidth3D;
		ImageHeight = CSImageHeight3D;
		ImageDepth = CSImageDepth3D;
		aCountImageWidth = areaCountImageWidth3D;
		aCountImageHeight = areaCountImageHeight3D;
		aCertainCount_x = areaCertainCount_x_3D;
		aCertainCount_y = areaCertainCount_y_3D;
		aCertainCount_z = areaCertainCount_z_3D;
		break;
	default:
		break;
	}
	//std::string fname1 = "res/test1.csv";
	//std::ofstream outFile1(fname1, std::ios::out);
	//for (int i = 0; i < CSImageWidth * CSImageHeight; i++) {
	//	outFile1<< std::to_string(data[i].x) << "," << std::to_string(data[i].y) << "," << std::to_string(data[i].u) << "," << std::to_string(data[i].v) << std::endl;
	//}
	//outFile1.close();
	//
	std::sort(data, data + ImageWidth * ImageHeight * ImageDepth, cmp);//将球按照区域位置从小到大排序
	//
	//std::string fname2 = "res/test2.csv";
	//std::ofstream outFile2(fname2, std::ios::out);
	//for (int i = 0; i < CSImageWidth * CSImageHeight; i++) {
	//	outFile2 << std::to_string(data[i].x) << "," << std::to_string(data[i].y) << "," << std::to_string(data[i].u) << "," << std::to_string(data[i].v) << std::endl;
	//}
	//outFile2.close();

	//float gg = data[0].u;
	//float g = data[0].v;

	Point* startIndexData = new Point[aCountImageWidth * aCountImageHeight];
	int index = 0;
	int area_index = data[index].i;//areaBias_x + areaBias_y * areaCount_x + areaBias_z * areaCount_x * areaCount_y; ////494646  
	int next_area_index = area_index;
	int pointCount = 0;
	bool stop = false;
	for (int i = 0; i < aCertainCount_x * aCertainCount_y * aCertainCount_z; ++i) { //记录每个区域对应的球起始索引和球数量
		pointCount = 0;
		startIndexData[area_index].i = index;
		while (next_area_index == area_index) {
			index++;
			pointCount++;
			if (index > ImageWidth * ImageHeight * ImageDepth - 1) {
				stop = true;
			}
			next_area_index = data[index].i;//下一个球所属的区域
		}
		startIndexData[area_index].u = pointCount;
		startIndexData[area_index].v = 0.0f;
		area_index = next_area_index;
		if (stop) break;
	}

	//std::string fname3 = "res/test4.csv";
	//std::ofstream outFile3(fname3, std::ios::out);
	//for (int i = 0; i < areaCount_x * areaCount_y; i++) {
	//	outFile3 << std::to_string(startIndexData[i].x) << "," << std::to_string(startIndexData[i].y) << "," << std::to_string(startIndexData[i].u) << "," << std::to_string(startIndexData[i].v) << std::endl;
	//}
	//outFile3.close();

	areaOutputStartIndexImage.TransferDataToTextureFloat((float*)startIndexData);
	areaOutputImage.TransferDataToTextureFloat((float*)data);
	delete[](startIndexData);
	delete[](data);

}

void InitialData(PointMode mode, Texture& pointInputImage,Texture& areaInputImage,Texture& areaInputStartIndexImage) {

	int ImageWidth;
	int ImageHeight;
	int ImageDepth;
	int aBias_x;
	int aBias_y;
	int aBias_z;
	int aCount_x;
	int	aCount_y;
	int	aCount_z;

	switch (mode)
	{
	case PointMode::xy:
		ImageWidth = CSImageWidth;
		ImageHeight = CSImageHeight;
		ImageDepth = CSImageDepth;
		aBias_x = areaBias_x;
		aBias_y = areaBias_y;
		aBias_z = areaBias_z;
		aCount_x = areaCount_x;
		aCount_y = areaCount_y;
		aCount_z = areaCount_z;
		break;
	case PointMode::xyz:
		ImageWidth = CSImageWidth3D;
		ImageHeight = CSImageHeight3D;
		ImageDepth = CSImageDepth3D;
		aBias_x = areaBias_x_3D;
		aBias_y = areaBias_y_3D;
		aBias_z = areaBias_z_3D;
		aCount_x = areaCount_x_3D;
		aCount_y = areaCount_y_3D;
		aCount_z = areaCount_z_3D;
		break;
	default:
		break;
	}

	float* inputData = new float[ImageWidth * ImageHeight * ImageDepth * 4 * 2];
	float* areaData = new float[ImageWidth * ImageHeight * ImageDepth * 4];
	glm::vec3 gap = glm::vec3(CSSpaceWidth / ImageWidth, CSSpaceHeight / ImageHeight, CSSpaceDepth / ImageDepth);
	float uv_z;
	int uv_xy_i;
	float uv_x;
	float uv_y;
	glm::vec3 position;
	int area_x;
	int area_y;
	int area_z;
	for (int i = 0; i < ImageWidth * ImageHeight * ImageDepth * 4 * 2; ++i) {
		if (i < ImageWidth * ImageHeight * ImageDepth * 4) {
			uv_xy_i = i / 4 % (ImageWidth * ImageHeight);
			uv_z = i / 4 / (ImageWidth * ImageHeight);
			uv_x = uv_xy_i % ImageWidth;
			uv_y = uv_xy_i / ImageWidth;
			position = glm::vec3(gap.x / 2.0f + uv_x * gap.x - CSSpaceWidth / 2.0f, gap.y / 2.0f + uv_y * gap.y - CSSpaceHeight / 2.0f, gap.z / 2.0f + uv_z * gap.z - CSSpaceDepth / 2.0f);//坐标

			area_x = int((position.x + CSSpaceWidth / 2.0f) / SmoothingRadius + aBias_x);
			area_y = int((position.y + CSSpaceHeight / 2.0f) / SmoothingRadius + aBias_y);
			area_z = int((position.z + CSSpaceDepth / 2.0f) / SmoothingRadius + aBias_z);

			inputData[i] = position.x;
			areaData[i] = area_x + area_y * aCount_x + area_z * aCount_x * aCount_y;

			inputData[++i] = position.y;
			areaData[i] = 0.0f;

			inputData[++i] = position.z;
			areaData[i] = uv_xy_i;

			inputData[++i] = 0.0f;
			areaData[i] = uv_z;
		}
		else
		{
			inputData[i] = 0.0f;
			inputData[++i] = 0.0f;
			inputData[++i] = 0.0f;
			inputData[++i] = 1.0f;
		}

	}

	//std::string fname3 = "res/test5.csv";
	//std::ofstream outFile3(fname3, std::ios::out);
	//for (int i = 0; i < CSImageWidth * CSImageHeight * CSImageDepth * 4 * 2; i++) {
	//	//outFile3 << std::to_string(inputData[i]) << "," << std::to_string(inputData[++i]) << "," << std::to_string(inputData[++i]) << "," << std::to_string(inputData[++i]) << std::endl;
	//	std::cout << std::to_string(inputData[i]) << "," << std::to_string(inputData[++i]) << "," << std::to_string(inputData[++i]) << "," << std::to_string(inputData[++i]) << std::endl;
	//
	//}
	//outFile3.close();

	pointInputImage.TransferDataToTextureFloat(inputData);
	areaInputImage.TransferDataToTextureFloat(areaData);
	//排序areaInputImage
	SortAreaOutputImage(mode,areaInputImage, areaInputStartIndexImage);

	delete[](inputData);
	delete[](areaData);
}

void Reset(PointMode mode, Texture& pointInputImage, Texture& areaInputImage, Texture& areaInputStartIndexImage, std::vector<glm::mat4>& modelMatrixList) {
	switch (mode)
	{
	case PointMode::xy:
		{
			CSSpaceWidth = SPACE_X;
			CSSpaceHeight = SPACE_Y;
			CSSpaceDepth = SPACE_Z;

			gravity = 0.0f;

			SmoothingRadiusMax = (CSSpaceWidth / 2.0f);
			SmoothingRadius = glm::min(SmoothingRadiusDefault, SmoothingRadiusMax);

			forceSize = FORCE_SET_SCALE;
			targetDensity = 250.0f;
			viscositySize = 0.0f;
			forceNearSize = FORCE_SET_SCALE / 10.0f;

			areaCertainCount_x = ceil(CSSpaceWidth / SmoothingRadius);
			areaCertainCount_y = ceil(CSSpaceHeight / SmoothingRadius);
			areaCertainCount_z = 1;
			areaBias_x = (areaCount_x - areaCertainCount_x) / 2.0f;
			areaBias_y = (areaCount_y - areaCertainCount_y) / 2.0f;
			areaBias_z = (areaCount_z - areaCertainCount_z) / 2.0f;

			MaxVelocity = 10.0f;

			InitialData(PointMode::xy, pointInputImage, areaInputImage, areaInputStartIndexImage);

			modelMatrixList.resize(CSImageWidth * CSImageHeight * CSImageDepth);

			break;

		}


	case PointMode::xyz:
		{
			CSSpaceWidth = SPACE_X_3D;
			CSSpaceHeight = SPACE_Y_3D;
			CSSpaceDepth = SPACE_Z_3D;

			gravity = 0.0f;

			SmoothingRadiusMax = (CSSpaceWidth / 2.0f);
			SmoothingRadius = glm::min(SmoothingRadiusDefault3D, SmoothingRadiusMax);

			forceSize = FORCE_SET_SCALE;
			targetDensity = 250.0f;
			viscositySize = 0.0f;
			forceNearSize = FORCE_SET_SCALE / 10.0f;

			areaCertainCount_x_3D = ceil(CSSpaceWidth / SmoothingRadius);
			areaCertainCount_y_3D = ceil(CSSpaceHeight / SmoothingRadius);
			areaCertainCount_z_3D = ceil(CSSpaceHeight / SmoothingRadius);;
			areaBias_x_3D = (areaCount_x_3D - areaCertainCount_x_3D) / 2.0f;
			areaBias_y_3D = (areaCount_y_3D - areaCertainCount_y_3D) / 2.0f;
			areaBias_z_3D = (areaCount_z_3D - areaCertainCount_z_3D) / 2.0f;

			MaxVelocity = 10.0f;

			InitialData(PointMode::xyz, pointInputImage, areaInputImage, areaInputStartIndexImage);

			modelMatrixList.resize(CSImageWidth3D * CSImageHeight3D * CSImageDepth3D);

			break;
		}

		default:
			break;
	}

}

void DrawSpace(Shader& shader, glm::vec3 MinPos, glm::vec3 MaxPos)
{
	float XMin = MinPos.x;
	float XMax = MaxPos.x;
	float YMin = MinPos.y;
	float YMax = MaxPos.y;
	float ZMin = MinPos.z;
	float ZMax = MaxPos.z;

	float AABBVertices[] = {
		// positions          
		XMin, YMax, ZMin,
		XMin, YMin, ZMin,
		XMax, YMin, ZMin,
		XMax, YMin, ZMin,
		XMax, YMax, ZMin,
		XMin, YMax, ZMin,

		XMin, YMin, ZMax,
		XMin, YMin, ZMin,
		XMin, YMax, ZMin,
		XMin, YMax, ZMin,
		XMin, YMax, ZMax,
		XMin, YMin, ZMax,

		XMax, YMin, ZMin,
		XMax, YMin, ZMax,
		XMax, YMax, ZMax,
		XMax, YMax, ZMax,
		XMax, YMax, ZMin,
		XMax, YMin, ZMin,

		XMin, YMin, ZMax,
		XMin, YMax, ZMax,
		XMax, YMax, ZMax,
		XMax, YMax, ZMax,
		XMax, YMin, ZMax,
		XMin, YMin, ZMax,

		XMin, YMax, ZMin,
		XMax, YMax, ZMin,
		XMax, YMax, ZMax,
		XMax, YMax, ZMax,
		XMin, YMax, ZMax,
		XMin, YMax, ZMin,

		XMin, YMin, ZMin,
		XMin, YMin, ZMax,
		XMax, YMin, ZMin,
		XMax, YMin, ZMin,
		XMin, YMin, ZMax,
		XMax, YMin, ZMax
	};
	unsigned int vaID;//VertexArray
	unsigned int vbID;

	VertexArray va(vaID);
	VertexBuffer vb(vbID, AABBVertices, 108 * sizeof(float));

	VertexAttribLayout layout;//创建顶点属性布局实例
	layout.Push<GL_FLOAT>(3);//填入第一个属性布局，类型为float，每个点为3维向量

	va.AddBuffer(vbID, layout);//将所有属性布局应用于顶点缓冲区vb，并绑定在顶点数组对象va上

	va.Unbind();
	vb.Unbind();

	shader.Bind();
	va.Bind();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	shader.Unbind();
	va.Unbind();
}

//加载模型
void LoadModel(std::unordered_map<std::string, Model>& modelMap, std::unordered_map<std::string, InstanceBuffer>& instanceMap) {
	modelMap.clear();
	std::string fileName = "sphere.obj";
	std::string filePath = "res/models/";
	Model model(filePath + fileName);//读取文件
	model.SetDefaultModelMatrix();
	modelMap[fileName] = model;

	fileName = "plane.obj";
	Model plane(filePath + fileName);//读取文件
	plane.SetDefaultModelMatrix();
	modelMap[fileName] = plane;

	//创建实例
	instanceMap.clear();

	modelMatrixList.resize(CSImageWidth * CSImageHeight * CSImageDepth);
	
	InstanceBuffer instance(CSImageWidth3D*CSImageHeight3D*CSImageDepth3D * sizeof(glm::mat4), modelMatrixList.data());
	instance.AddInstanceBuffermat4(modelMap["sphere.obj"].meshes[0].vaID, 3);
	instanceMap["sphere.obj"] = instance;

	InstanceBuffer instancePlane(sizeof(glm::mat4), &planeModelMatrix);
	instancePlane.AddInstanceBuffermat4(modelMap["plane.obj"].meshes[0].vaID, 3);
	instanceMap["plane.obj"] = instancePlane;
	
}

template<typename T>
T Lerp(T x, T y, float t)
{
	T temp = t * y + (1 - t) * x;
	return temp;
}




// Main code
#ifdef DEBUG
int main(int, char**)
#else
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
	//glfw初始化
    if (!glfwInit())
        return 1;
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_SAMPLES, 8);//设置MSAAx8
    //glEnable(GL_MULTISAMPLE);//开启MSAA

    //创建窗口上下文
    GLFWwindow* window = glfwCreateWindow(WinWidth, WinHeight, "App", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }       
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); //1=开启垂直同步

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//LONG_PTR exst = ::GetWindowLongPtr(Handle, GWL_EXSTYLE);
	//::SetWindowLongPtr(Handle, GWL_EXSTYLE, exst | WS_EX_ACCEPTFILES);

    //glew初始化
	if (glewInit())
	{
		return 1;
	}
    
	std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;

    //初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    io.Fonts->AddFontFromFileTTF("res/fonts/msyh.ttc", 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());//中文字体
    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.73f, 0.73f, 0.71f, 0.39f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.78f, 0.35f, 0.59f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.78f, 0.35f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.98f, 0.98f, 0.99f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.98f, 0.98f, 0.99f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.73f, 0.73f, 0.75f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.73f, 0.73f, 0.75f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.73f, 0.73f, 0.75f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.78f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
	colors[ImGuiCol_Button] = ImVec4(0.73f, 0.73f, 0.71f, 0.39f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.54f, 0.76f, 1.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.41f, 0.71f, 1.00f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.73f, 0.73f, 0.75f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.73f, 0.73f, 0.75f, 0.59f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.73f, 0.73f, 0.75f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.66f, 0.66f, 0.68f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.35f, 0.35f, 0.35f, 0.17f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.88f, 0.88f, 0.90f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.88f, 0.88f, 0.90f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.22f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	style.FrameRounding = 5.0f;
	style.FrameBorderSize = 1.0f;
	style.WindowRounding = 5.0f;
	style.PopupRounding = 5.0f;
	style.FramePadding = ImVec2(6.0f, 6.0f);
	style.WindowMenuButtonPosition = 1;


	//模型哈希表(毫米)
	std::unordered_map<std::string, Model> modelMap;

    //实例哈希表
    std::unordered_map<std::string, InstanceBuffer> instanceMap;

	//加载模型
	LoadModel(modelMap, instanceMap);


	//创建变换矩阵
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;


	//设置直射光VP变换矩阵
	glm::mat4 LightProjectionMatrix = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.01f, 100.0f);
	glm::mat4 LightViewMatrix = glm::lookAt(glm::vec3(20.0f,20.0f,0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	
    //Shader
    Shader shader("res/shaders/Basic.shader");
	Shader depthShader("res/shaders/Depth.shader");
	Shader densityDisplayShader("res/shaders/DensityDisplay.shader");
	Shader displayShader("res/shaders/BasicDisplay.shader");
	Shader planeShader("res/shaders/Plane.shader");
	Shader spaceShader("res/shaders/Space.shader");

	Shader computeShader("res/shaders/ComputeBasic.shader");
	Shader computeShader3D("res/shaders/3DComputeBasic.shader");
	Shader computeRefreshShader("res/shaders/ComputeRefresh.shader");
	Shader computeRefreshShader3D("res/shaders/3DComputeRefresh.shader");
	Shader computeRefreshAreaStartIndexImageShader("res/shaders/ComputeRefreshAreaStartIndexImage.shader");
	Shader computeRefreshAreaStartIndexImageShader3D("res/shaders/3DComputeRefreshAreaStartIndexImage.shader");
	Shader computeRefreshInputImageShader("res/shaders/ComputeRefreshInputImage.shader");
	Shader computeRefreshInputImageShader3D("res/shaders/3DComputeRefreshInputImage.shader");
	//Shader computeDensityShader("res/shaders/ComputeDensity.shader");
	//Shader computeClearShader("res/shaders/ComputeClear.shader");
	Shader computeDensityPredictShader("res/shaders/ComputeDensityPredict.shader");
	Shader computeDensityPredictShader3D("res/shaders/3DComputeDensityPredict.shader");
	Shader computePositionPredictShader("res/shaders/ComputePositionPredict.shader");
	Shader computePositionPredictShader3D("res/shaders/3DComputePositionPredict.shader");

	Texture pointInputImage(CSImageWidth * CSImageHeight, CSImageDepth * 2);//posx,posy,posz  //vx,vy,vz
	Texture pointInputImage3D(CSImageWidth3D * CSImageHeight3D, CSImageDepth3D * 2);//posx,posy,posz  //vx,vy,vz
	Texture pointOutputImage(CSImageWidth * CSImageHeight, CSImageDepth * 2);//posx,posy,posz  //vx,vy,vz
	Texture pointOutputImage3D(CSImageWidth3D * CSImageHeight3D, CSImageDepth3D * 2);//posx,posy,posz  //vx,vy,vz
	Texture parameterImage(CSParameterWidth, CSParameterHeight);
	float* parameterData = new float[CSParameterWidth * CSParameterHeight * 4];

	//Texture densityImage(CSImageWidth, CSImageWidth);//密度图
	Texture predictDensityImage(CSImageWidth * CSImageWidth, CSImageDepth);//预测密度图 //predict_density,predict_neardensity
	Texture predictDensityImage3D(CSImageWidth3D* CSImageWidth3D, CSImageDepth3D);//预测密度图 //predict_density,predict_neardensity

	Texture areaInputImage(CSImageWidth* CSImageHeight, CSImageDepth);//所属区域数组 i,0,u,v
	Texture areaInputImage3D(CSImageWidth3D* CSImageHeight3D, CSImageDepth3D);//所属区域数组 i,0,u,v
	Texture areaOutputImage(CSImageWidth* CSImageHeight, CSImageDepth);//所属区域数组 i,0,u,v
	Texture areaOutputImage3D(CSImageWidth3D* CSImageHeight3D, CSImageDepth3D);//所属区域数组 i,0,u,v

	Texture areaInputStartIndexImage(areaCountImageWidth, areaCountImageHeight);//区域起始索引数组 x,y,count opengl报错
	Texture areaInputStartIndexImage3D(areaCountImageWidth3D, areaCountImageHeight3D);//区域起始索引数组 x,y,count opengl报错
	Texture areaOutputStartIndexImage(areaCountImageWidth, areaCountImageHeight);//区域起始索引数组 x,y,count
	Texture areaOutputStartIndexImage3D(areaCountImageWidth3D, areaCountImageHeight3D);//区域起始索引数组 x,y,count
	std::cout << std::to_string(GLCheckError()) << std::endl;


	Texture predictAreaOutputImage(CSImageWidth* CSImageHeight, CSImageDepth);//预测后的所属区域数组 i,gg,u,v
	Texture predictAreaOutputImage3D(CSImageWidth3D* CSImageHeight3D, CSImageDepth3D);//预测后的所属区域数组 i,gg,u,v
	Texture predictAreaOutputStartIndexImage(areaCountImageWidth, areaCountImageHeight);//预测后的区域起始索引数组 x,y,count
	Texture predictAreaOutputStartIndexImage3D(areaCountImageWidth3D, areaCountImageHeight3D);//预测后的区域起始索引数组 x,y,count

	Texture predictPointOutputImage(CSImageWidth * CSImageHeight, CSImageDepth * 2); //posx,posy,posz //vx,vy,vz
	Texture predictPointOutputImage3D(CSImageWidth3D * CSImageHeight3D, CSImageDepth3D * 2); //posx,posy,posz //vx,vy,vz


	InitialData(PointMode::xy, pointInputImage, areaInputImage, areaInputStartIndexImage);

	FrameBuffer display(DisplayWidth, DisplayHeight);
	display.GenTexture2D();

	//FrameBuffer density(DensityDisplayWidth, DensityDisplayHeight);
	//density.GenTexture2D();
	//unsigned int QuadID = density.GenQuad();

	//shadowmap
	FrameBuffer framebufferSM(ShadowMapWidth, ShadowMapHeight);
	framebufferSM.GenTexture2DShadowMap();


	//渲染方面的API
	Renderer renderer;

	//创建Uniform缓冲对象
	UniformBuffer ubo(2 * sizeof(glm::mat4), 0);
	std::vector<int> shaderIDs;
	shaderIDs.push_back(shader.RendererID);
	shaderIDs.push_back(depthShader.RendererID);
	shaderIDs.push_back(planeShader.RendererID);
	shaderIDs.push_back(spaceShader.RendererID);
    ubo.Bind(shaderIDs, "Matrices");

   

    //主循环
    while (!glfwWindowShouldClose(window))
    {
		GLClearError();//清除错误信息

		//记录每帧的时间
		deltaTime = (float)glfwGetTime() - lastTime;
		lastTime = (float)glfwGetTime();

		//forceSize = Lerp(forceSize, FORCE_SET_SCALE, deltaTime / TotalTime);
		//forceNearSize = forceSize;
		forceNearSize = forceSize / 10.0f;

		SmoothingRadiusMax = (CSSpaceWidth / 2.0f);

		switch (pointMode)
		{
		case PointMode::xy:
			areaCertainCount_x = ceil(CSSpaceWidth / SmoothingRadius);
			areaCertainCount_y = ceil(CSSpaceHeight / SmoothingRadius);
			areaCertainCount_z = 1;
			areaBias_x = (areaCount_x - areaCertainCount_x) / 2.0f;
			areaBias_y = (areaCount_y - areaCertainCount_y) / 2.0f;
			areaBias_z = (areaCount_z - areaCertainCount_z) / 2.0f;
			break;
		case PointMode::xyz:
			areaCertainCount_x_3D = ceil(CSSpaceWidth / SmoothingRadius);
			areaCertainCount_y_3D = ceil(CSSpaceHeight / SmoothingRadius);
			areaCertainCount_z_3D = ceil(CSSpaceDepth / SmoothingRadius);
			areaBias_x_3D = (areaCount_x_3D - areaCertainCount_x_3D) / 2.0f;
			areaBias_y_3D = (areaCount_y_3D - areaCertainCount_y_3D) / 2.0f;
			areaBias_z_3D = (areaCount_z_3D - areaCertainCount_z_3D) / 2.0f;
			break;
		default:
			break;
		}
		

		//相机键盘输入控制
		if (mouseRole == MouseRole::ControlCamera) {
			camera.KeyControl(window, deltaTime);
		}
           



		//设置变换矩阵			
		//modelMatrix = glm::rotate(modelMatrix, deltaTime * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));//旋转		
		viewMatrix = camera.SetView();
		projectionMatrix = camera.SetProjection((float)DisplayWidth / DisplayHeight);

		//将model矩阵数组填入实例哈希表
		for (glm::mat4& modelMatrix : modelMatrixList) {
			modelMatrix = modelMap["sphere.obj"].GetModelMatrix();
		}
		instanceMap["sphere.obj"].RefreshDatamat4(sizeof(glm::mat4)* modelMatrixList.size(), modelMatrixList.data());

		planeModelMatrix = modelMap["plane.obj"].GetDefaultModelMatrix();
		planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f,-CSSpaceHeight/2.0f,0.0f));
		instanceMap["plane.obj"].RefreshDatamat4(sizeof(glm::mat4), &planeModelMatrix);
		

		//向uniform缓冲对象填入view、projection矩阵数据
		ubo.SetDatamat4(0, sizeof(glm::mat4), &viewMatrix);
		ubo.SetDatamat4(sizeof(glm::mat4), sizeof(glm::mat4), &projectionMatrix);

		//更新参数
		//(0,0)
		parameterData[0] = deltaTime;//秒
		parameterData[1] = CSSpaceWidth;
		parameterData[2] = CSSpaceHeight;
		parameterData[3] = CSSpaceDepth;
		//(1,0)
		parameterData[7] = SmoothingRadius;
		//(2,0)
		parameterData[8] = targetDensity;
		parameterData[9] = viscositySize* VISCOSITY_SCALE;
		parameterData[10] = forceNearSize;
		parameterData[11] = gravity;
		//(3,0)
		parameterData[12] = forceSize;
		switch (pointMode)
		{
		case PointMode::xy:
			parameterData[4] = areaBias_x;
			parameterData[5] = areaBias_y;
			parameterData[6] = areaBias_z;

			parameterData[13] = areaCount_x;
			parameterData[14] = areaCount_y;
			parameterData[15] = areaCount_z;
			//(4,0)
			parameterData[16] = CSImageWidth;
			parameterData[17] = CSImageHeight;
			parameterData[18] = CSImageDepth;
			parameterData[19] = BorderForce;
			break;
		case PointMode::xyz:
			parameterData[4] = areaBias_x_3D;
			parameterData[5] = areaBias_y_3D;
			parameterData[6] = areaBias_z_3D;

			parameterData[13] = areaCount_x_3D;
			parameterData[14] = areaCount_y_3D;
			parameterData[15] = areaCount_z_3D;
			//(4,0)
			parameterData[16] = CSImageWidth3D;
			parameterData[17] = CSImageHeight3D;
			parameterData[18] = CSImageDepth3D;
			parameterData[19] = BorderForce;
			break;
		default:
			break;
		}
		
		

		parameterImage.TransferDataToTextureFloat(parameterData);

		//pass-1
// 		positionOutputImage.BindComputeReadWrite(0);
// 		computeClearShader.BindCompute(Group_Density_a, Group_Density_b, Group_Density_c);
// 		predictPositionOutputImage.BindComputeReadWrite(0);
// 		computeClearShader.BindCompute(Group_Density_a, Group_Density_b, Group_Density_c);

		//当前density
		//inputImage.BindComputeRead(0);
		//densityImage.BindComputeWrite(1);
		//parameterImage.BindComputeRead(2);
		//areaInputImage.BindComputeRead(3);
		//areaInputStartIndexImage.BindComputeRead(4);
		//computeDensityShader.BindCompute(Group_a, Group_b, Group_c);

		//预测position
		switch (pointMode)
		{
		case PointMode::xy:
			pointInputImage.BindComputeRead(0);
			predictPointOutputImage.BindComputeWrite(1);
			parameterImage.BindComputeRead(2);
			predictAreaOutputImage.BindComputeWrite(3);
			computePositionPredictShader.BindCompute(Group_a, Group_b, Group_c);

			//排序predictAreaOutputImage
			SortAreaOutputImage(pointMode, predictAreaOutputImage, predictAreaOutputStartIndexImage);//去掉后从20fps提升到70fps

			//预测density
			predictPointOutputImage.BindComputeRead(0);
			predictDensityImage.BindComputeWrite(1);
			parameterImage.BindComputeRead(2);
			predictAreaOutputImage.BindComputeRead(3);
			predictAreaOutputStartIndexImage.BindComputeRead(4);
			computeDensityPredictShader.BindCompute(Group_a, Group_b, Group_c);

			//更新
			pointInputImage.BindComputeRead(0);
			pointOutputImage.BindComputeWrite(1);
			parameterImage.BindComputeRead(2);
			predictDensityImage.BindComputeRead(3);//预测
			predictAreaOutputImage.BindComputeRead(4);//预测
			predictAreaOutputStartIndexImage.BindComputeRead(5);//预测
			areaOutputImage.BindComputeWrite(6);
			predictPointOutputImage.BindComputeRead(7);//预测
			computeShader.BindCompute(Group_a, Group_b, Group_c);

			//排序areaOutputImage
			SortAreaOutputImage(pointMode, areaOutputImage, areaOutputStartIndexImage);

			//交换新旧球集合
			pointOutputImage.BindComputeRead(0);
			pointInputImage.BindComputeWrite(1);
			parameterImage.BindComputeRead(2);
			computeRefreshInputImageShader.BindCompute(Group_a, Group_b, Group_c);

			//交换新旧position
			areaOutputImage.BindComputeRead(0);
			areaInputImage.BindComputeWrite(1);
			computeRefreshShader.BindCompute(Group_a, Group_b, Group_c);

			areaOutputStartIndexImage.BindComputeRead(0);
			areaInputStartIndexImage.BindComputeWrite(1);
			parameterImage.BindComputeRead(2);
			computeRefreshAreaStartIndexImageShader.BindCompute(areaCountImageWidth, areaCountImageHeight, 1);
			break;
		case PointMode::xyz:
			pointInputImage3D.BindComputeRead(0);
			predictPointOutputImage3D.BindComputeWrite(1);
			parameterImage.BindComputeRead(2);
			predictAreaOutputImage3D.BindComputeWrite(3);
			computePositionPredictShader3D.BindCompute(Group_a_3D, Group_b_3D, Group_c_3D);

			//排序predictAreaOutputImage
			SortAreaOutputImage(pointMode, predictAreaOutputImage3D, predictAreaOutputStartIndexImage3D);//去掉后从20fps提升到70fps

			//预测density
			predictPointOutputImage3D.BindComputeRead(0);
			predictDensityImage3D.BindComputeWrite(1);
			parameterImage.BindComputeRead(2);
			predictAreaOutputImage3D.BindComputeRead(3);
			predictAreaOutputStartIndexImage3D.BindComputeRead(4);
			computeDensityPredictShader3D.BindCompute(Group_a_3D, Group_b_3D, Group_c_3D);

			//更新
			pointInputImage3D.BindComputeRead(0);
			pointOutputImage3D.BindComputeWrite(1);
			parameterImage.BindComputeRead(2);
			predictDensityImage3D.BindComputeRead(3);//预测
			predictAreaOutputImage3D.BindComputeRead(4);//预测
			predictAreaOutputStartIndexImage3D.BindComputeRead(5);//预测
			areaOutputImage3D.BindComputeWrite(6);
			predictPointOutputImage3D.BindComputeRead(7);//预测
			computeShader3D.BindCompute(Group_a_3D, Group_b_3D, Group_c_3D);

			//排序areaOutputImage
			SortAreaOutputImage(pointMode, areaOutputImage3D, areaOutputStartIndexImage3D);

			//交换新旧球集合
			pointOutputImage3D.BindComputeRead(0);
			pointInputImage3D.BindComputeWrite(1);
			parameterImage.BindComputeRead(2);
			computeRefreshInputImageShader3D.BindCompute(Group_a_3D, Group_b_3D, Group_c_3D);

			//交换新旧position
			areaOutputImage3D.BindComputeRead(0);
			areaInputImage3D.BindComputeWrite(1);
			computeRefreshShader3D.BindCompute(Group_a_3D, Group_b_3D, Group_c_3D);

			areaOutputStartIndexImage3D.BindComputeRead(0);
			areaInputStartIndexImage3D.BindComputeWrite(1);
			parameterImage.BindComputeRead(2);
			computeRefreshAreaStartIndexImageShader3D.BindCompute(areaCountImageWidth3D, areaCountImageHeight3D, 1);
			break;
		default:
			break;
		}
		

		//pass0

		//ShadowMap
		glViewport(0, 0, ShadowMapWidth, ShadowMapHeight);//修改视口尺寸
		//将光源当作正交摄像头的VP变换矩阵，M矩阵当然还是沿用各物体的。
		
		depthShader.Bind();
		depthShader.SetUniformMat4("view", LightViewMatrix);
		depthShader.SetUniformMat4("projection", LightProjectionMatrix);
		switch (pointMode)
		{
		case PointMode::xy:
			pointInputImage.BindComputeRead(0);
			break;
		case PointMode::xyz:
			pointInputImage3D.BindComputeRead(0);
			break;
		default:
			break;
		}
		parameterImage.BindComputeRead(1);
		framebufferSM.Bind();//绑定帧缓冲对象，接收深度
		renderer.ClearDepth();//只需清除深度，不需清除颜色
		renderer.CullFace(0);
		
		switch (pointMode)
		{
		case PointMode::xy:
			modelMap["sphere.obj"].DrawInstanced(depthShader, CSImageWidth * CSImageHeight * CSImageDepth);
			break;
		case PointMode::xyz:
			modelMap["sphere.obj"].DrawInstanced(depthShader, CSImageWidth3D * CSImageHeight3D * CSImageDepth3D);
			break;
		default:
			break;
		}
		
		depthShader.Unbind();
		framebufferSM.Unbind();
		
		glViewport(0, 0, WinWidth, WinHeight);//还原视口尺寸


		//pass1
		glViewport(0, 0, DisplayWidth, DisplayHeight);//修改视口尺寸
		display.Bind();//framebuffer
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		renderer.ClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClearDepth(1.0);
		renderer.ClearDepth();
		
		renderer.CullFace(0);	

		//球
		shader.Bind();
		switch (pointMode)
		{
		case PointMode::xy:
			pointInputImage.BindComputeRead(0);
			predictDensityImage.BindComputeRead(2);
			break;
		case PointMode::xyz:
			pointInputImage3D.BindComputeRead(0);
			predictDensityImage3D.BindComputeRead(2);
			break;
		default:
			break;
		}
		parameterImage.BindComputeRead(1);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, framebufferSM.GetTexID());
		shader.SetUniform1i("shadowmap", 5);
		shader.SetUniformMat4("view", LightViewMatrix);
		shader.SetUniformMat4("projection", LightProjectionMatrix);
		shader.SetUniform1f("MaxVelocity", MaxVelocity);
		shader.SetUniform1i("VisualMode", (int)visualMode);
		switch (pointMode)
		{
		case PointMode::xy:
			modelMap["sphere.obj"].DrawInstanced(shader, CSImageWidth * CSImageHeight * CSImageDepth);
			break;
		case PointMode::xyz:
			modelMap["sphere.obj"].DrawInstanced(shader, CSImageWidth3D * CSImageHeight3D * CSImageDepth3D);
			break;
		default:
			break;
		}
		shader.Unbind();

		//边框
		DrawSpace(spaceShader, glm::vec3(-CSSpaceWidth / 2.0f, -CSSpaceHeight / 2.0f, -CSSpaceDepth / 2.0f), glm::vec3(CSSpaceWidth / 2.0f, CSSpaceHeight / 2.0f, CSSpaceDepth / 2.0f));

		//平面
		planeShader.Bind();
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, framebufferSM.GetTexID());
		planeShader.SetUniform1i("shadowmap", 5);
		planeShader.SetUniformMat4("view", LightViewMatrix);
		planeShader.SetUniformMat4("projection", LightProjectionMatrix);
		modelMap["plane.obj"].Draw(planeShader);
		planeShader.Unbind();

		display.Unbind();//framebuffer
		glViewport(0, 0, WinWidth, WinHeight);//还原视口尺寸


		//pass2
		//density.Bind();
		//densityDisplayShader.Bind();
		//predictDensityImage.BindComputeRead(0);//densityImage 
		//densityDisplayShader.SetUniform1f("DensityDisplayWidth", DensityDisplayWidth);
		//densityDisplayShader.SetUniform1f("DensityDisplayHeight", DensityDisplayHeight);
		////float bound = int(SmoothingRadius / glm::max(CSSpaceWidth, CSSpaceHeight) * glm::max(CSDensityImageWidth, CSDensityImageHeight));
		//densityDisplayShader.SetUniform1f("MaxDensity", SmoothingRadius* SmoothingRadius/((PI * pow(SmoothingRadius, 4.0f)) / 6.0f));
		//densityDisplayShader.SetUniform1f("TargetDensity", targetDensity);
		//density.Draw(densityDisplayShader, QuadID);
		//density.Unbind();

		//pass3
// 		position.Bind();
// 		displayShader.Bind();
// 		positionOutputImage.BindComputeRead(0);//predictPositionOutputImage
// 		displayShader.SetUniform1f("DisplayWidth", DensityDisplayWidth);
// 		displayShader.SetUniform1f("DisplayHeight", DensityDisplayHeight);
// 		displayShader.SetUniform1f("MaxSize", 1.0f);
// 		position.Draw(displayShader, QuadID1);
// 		position.Unbind();
		

        //2.渲染ImGui界面
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		App.ShowMyApp();

		ImGui::Begin("显示");
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Image((GLuint*)display.GetTexID(), ImVec2(DisplayWidth, DisplayHeight), ImVec2(0, 1),  ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();
		vMin.x += ImGui::GetWindowPos().x;
		vMin.y += ImGui::GetWindowPos().y;
		vMax.x += ImGui::GetWindowPos().x;
		vMax.y += ImGui::GetWindowPos().y;
		//ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(255, 255, 0, 255));
		if (ImGui::GetMousePos().x< vMax.x&& ImGui::GetMousePos().x>vMin.x&& ImGui::GetMousePos().y< vMax.y && ImGui::GetMousePos().y>vMin.y) {
			mouseState = MouseState::InDisplay;
		}
		else {
			mouseState = MouseState::OutOfDisplay;
		}
		
		ImGui::End();		


        ImGui::Begin("Debug");
        //获取OpenGL错误信息	
		//ImGui::Text(("OpenGL: " + std::to_string(GLCheckError())).c_str());
		//ImGui::Image((GLuint*)density.GetTexID(), ImVec2(DensityDisplayWidth, DensityDisplayHeight), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		//ImGui::Image((GLuint*)position.GetTexID(), ImVec2(DensityDisplayWidth, DensityDisplayHeight), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		if (ImGui::Button("加载2D流体")) {
			pointMode = PointMode::xy;
			Reset(pointMode, pointInputImage, areaInputImage, areaInputStartIndexImage, modelMatrixList);
		}
		ImGui::SameLine();
		if (ImGui::Button("加载3D流体")) {
			pointMode = PointMode::xyz;
			Reset(pointMode, pointInputImage3D, areaInputImage3D, areaInputStartIndexImage3D, modelMatrixList);
		}
// 		CSSpaceLastWidth = CSSpaceWidth;
// 		CSSpaceLastHeight = CSSpaceHeight;
// 		CSSpaceLastWidthVelocity = CSSpaceWidthVelocity;
// 		CSSpaceLastHeightVelocity = CSSpaceHeightVelocity;
		ImGui::DragFloat("边界宽", &CSSpaceWidth, 0.1f, SPACE_MIN_SCALE, SPACE_MAX_SCALE, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("边界高", &CSSpaceHeight, 0.1f, SPACE_MIN_SCALE, SPACE_MAX_SCALE, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		if(pointMode == PointMode::xyz) ImGui::DragFloat("边界深", &CSSpaceDepth, 0.1f, SPACE_MIN_SCALE, SPACE_MAX_SCALE, "%.3f", ImGuiSliderFlags_AlwaysClamp);
// 		CSSpaceWidthVelocity = (CSSpaceWidth - CSSpaceLastWidth) / deltaTime;
// 		CSSpaceHeightVelocity = (CSSpaceHeight - CSSpaceLastHeight) / deltaTime;
		ImGui::DragFloat("重力大小", &gravity, 0.01f, GRAVITY_MIN_SCALE, GRAVITY_MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("密度采样大小", &SmoothingRadius, 0.01f, SmoothingRadiusMin, SmoothingRadiusMax, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("目标密度", &targetDensity, 10.0f, DENSITY_MIN_SCALE, DENSITY_MAX_SCALE, "%.0f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("密度力量大小", &forceSize, 0.1f, FORCE_MIN_SCALE, FORCE_MAX_SCALE, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("粘度力量大小", &viscositySize, 10.0f, VISCOSITY_MIN_SCALE, VISCOSITY_MAX_SCALE, "%.0f", ImGuiSliderFlags_AlwaysClamp);
		//ImGui::DragFloat("近密度力量大小", &forceNearSize, 0.0001f, FORCENEAR_MIN_SCALE, FORCENEAR_MAX_SCALE, "%.4f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::Text("可视化类型: ");
		ImGui::RadioButton("密度", (int*)&visualMode, (int)VisualMode::Density);
		ImGui::SameLine();
		ImGui::RadioButton("速度", (int*)&visualMode, (int)VisualMode::Velocity);
		if(visualMode == VisualMode::Velocity)
			ImGui::DragFloat("可视化最大速度", &MaxVelocity, 1.0f, VELOCITY_MIN_SCALE, VELOCITY_MAX_SCALE, "%.0f", ImGuiSliderFlags_AlwaysClamp);

		//ImGui::InputFloat("边界排斥力", &BorderForce);

		//float* data = predictDensityImage.GetDataFromTextureFloat(4);
		//for (int i = 0; i < CSImageWidth * CSImageHeight*4; i+=4) {
		//	ImGui::Text("%f", data[i]);
		//}
		//float* data = pointOutputImage.GetDataFromTextureFloat(4);
		//for (int i = 0; i < CSImageWidth * CSImageHeight * 4; i+=4) {
		//	if(data[i]>0.9) ImGui::Text("%f,%f", data[i], data[i+1]);
		//}
		//float* data = positionOutputImage.GetDataFromTextureFloat(4);
		//for (int i = 0; i < CSDensityImageWidth*CSDensityImageHeight*4; i+=4) {
		//	ImGui::Text("%f", data[i]);
		//}
		//float* data = predictOutputImage.GetDataFromTextureFloat(4);
		//for (int i = 0; i < CSImageWidth * CSImageHeight * 4; i++) {
		//	ImGui::Text("%f", data[i]);
		//}
		

		ImGui::End();


		//ImGui::Begin("三维检索");
		//ImGui::End();


        //Rendering       
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(WinWidth, WinHeight);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        //3.一帧结束
        glfwPollEvents();
        glfwSwapBuffers(window);

        
    }

    //销毁

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	if (mouseRole == MouseRole::ControlCamera) {
		camera.MouseControl(xposIn, yposIn);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (mouseRole == MouseRole::ControlCamera) {
		camera.ScrollControl(xoffset, yoffset);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
 	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && mouseState == MouseState::InDisplay) {	
 		mouseRole = MouseRole::ControlCamera;
		camera.SetFirstMouse();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
 	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE && mouseRole == MouseRole::ControlCamera) {
		mouseRole = MouseRole::UI;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

}
