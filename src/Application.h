#pragma once
#include <unordered_map>
#include <map>
//#include <string>
//#include <atlstr.h>
//#include <memory>
//#include <mutex>
#include <regex>//正则表达式
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
// #include <thread>
// #include <mutex>
// #include <atomic>
// #include <omp.h>

#include <io.h> //获取目录文件名

//获取略缩图
#include <shobjidl_core.h>
#include <thumbcache.h>
#include <atlstr.h>

#include <Commdlg.h>//浏览文件


namespace MyApp {


	class MyApplication {
	private:
		
		bool toLoadProjectAddress = false;//是否弹出项目地址窗口
		bool toLoadPathSetting = false;//是否弹出工作目录设置窗口
		

		std::string CADName = "pole";//默认CAD文件名
		std::string CADType = ".SLDPRT";//默认CAD文件类型
		std::string CADPath = "C:\\Users\\PLY\\Desktop\\Files\\Projects\\SWApp\\SolidWorks Part\\";//默认CAD文件路径
		std::string CADTempPath = "C:\\Users\\PLY\\Desktop\\Files\\Projects\\SWApp\\SolidWorks Temp\\";//默认CAD临时文件路径
		std::string PictureExportPathForMBD = "C:\\Users\\PLY\\Desktop\\Files\\Projects\\Pycharm Projects\\MBDViewFeature\\MBDViewDataset\\photos\\";
		std::string PictureExportPathFornoMBD = "C:\\Users\\PLY\\Desktop\\Files\\Projects\\Pycharm Projects\\MBDViewFeature\\MBDViewDataset_noMBD\\photos\\";
		std::string ModelPictureExportPath = "C:\\Users\\PLY\\Desktop\\Files\\Projects\\Pycharm Projects\\MBDViewFeature\\MBDViewModelPicture\\";
		
		std::string PythonHome = "C:/Users/PLY/anaconda3/envs/torchgpu";
		std::string PythonProjectPath = "C:/Users/PLY/Desktop/Files/Projects/Pycharm Projects/MBDViewFeature";

		void EnableDocking();//开启Docking特性
		void ShowMenuBar();//显示菜单栏
		bool ShowMessage(const char* message);//显示ImGui弹窗，而ImGui::OpenPopup("提示")用于弹出弹窗
		bool ShowPathSetting();//显示ImGui弹窗，而ImGui::OpenPopup("工作路径设置")用于弹出弹窗

	public:
		void ShowMyApp();

		inline void SetCADPath(std::string path) { CADPath = path; };//获取保存模型时的路径
		inline void SetCADTempPath(std::string path) { CADTempPath = path; };//获取保存模型时的路径
		inline void SetPictureExportPathForMBD(std::string path) {PictureExportPathForMBD = path;}
		inline void SetPictureExportPathFornoMBD(std::string path) { PictureExportPathFornoMBD = path; }
		inline void SetModelPictureExportPath(std::string path) { ModelPictureExportPath = path; }
		inline void SetPythonHome(std::string path) { PythonHome = path; }
		inline void SetPythonProjectPath(std::string path) { PythonProjectPath = path; }

		

	//极致简洁的单例模式，运用了static的性质
	private:
		static std::shared_ptr<MyApplication> instance;
		
		MyApplication() {};
		~MyApplication() {};
	public:
		static MyApplication& GetInstance() {
			static MyApplication instance;
			return instance;
		}
		

	};



    
}
