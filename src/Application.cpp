#include "Application.h"
#include "imgui.h"




namespace MyApp {


	void MyApplication::ShowMyApp()
    {
		EnableDocking(); //开启Docking特性
		//ImGui::ShowDemoWindow();

        //ImGui::Begin("SolidWorks API");
		
        //ImGui::End();

    }





	void  MyApplication::EnableDocking()
    {
        
        static bool opt_fullscreen = true;//false时可关闭ImGui背景
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("SWApp", nullptr, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
        
		
		ShowMenuBar();
		

        ImGui::End();
    }


	void  MyApplication::ShowMenuBar()
	{
		
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu(" 关于 ")) {
				if (ImGui::MenuItem("项目地址")) {
					toLoadProjectAddress = true;
				}
				ImGui::EndMenu();

			}
			//if (ImGui::BeginMenu(" 设置 ")) {
			//	if (ImGui::MenuItem("工作路径")) {
			//		toLoadPathSetting = true;
			//	}
			//	ImGui::EndMenu();
			//
			//}
			ImGui::EndMenuBar();
		}


		if (toLoadProjectAddress)
		{
			ImGui::OpenPopup("提示");
			toLoadProjectAddress = ShowMessage("github.com/PLY2001/SPH-based-Viscoelastic-Fluid-Simulation");
		}
		//if (toLoadPathSetting)
		//{
		//	ImGui::OpenPopup("工作路径设置:config.ini");
		//	toLoadPathSetting = ShowPathSetting();
		//}
	}

	bool  MyApplication::ShowMessage(const char* message)
	{
		
		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("提示", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{


			ImGui::Text(message);

			ImGui::Separator();
			if (ImGui::Button("关闭", ImVec2(120, 0)))
			{

				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return false;

			}
			ImGui::EndPopup();
		}
		return true;
	}
	
	//bool MyApplication::ShowPathSetting()
	//{
	//	// Always center this window when appearing
	//	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	//	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	//
	//	if (ImGui::BeginPopupModal("工作路径设置:config.ini", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	//	{
	//		ImGui::Text("【SW零件库】");
	//		ImGui::SameLine();
	//		ImGui::Text(CADPath.c_str());
	//		ImGui::Text("【SW临时文件】");
	//		ImGui::SameLine();
	//		ImGui::Text(CADTempPath.c_str());
	//		ImGui::Text("【SW零件略缩图库】");
	//		ImGui::SameLine();
	//		ImGui::Text(ModelPictureExportPath.c_str());
	//		ImGui::Text("【MBD视图特征库】");
	//		ImGui::SameLine();
	//		ImGui::Text(PictureExportPathForMBD.c_str());
	//		ImGui::Text("【无MBD视图特征库】");
	//		ImGui::SameLine();
	//		ImGui::Text(PictureExportPathFornoMBD.c_str());
	//		ImGui::Text("【Python库】");
	//		ImGui::SameLine();
	//		ImGui::Text(PythonHome.c_str());
	//		ImGui::Text("【Python神经网络项目】");
	//		ImGui::SameLine();
	//		ImGui::Text(PythonProjectPath.c_str());
	//
	//		ImGui::Separator();
	//
	//
	//		if (ImGui::Button("关闭", ImVec2(120, 0)))
	//		{
	//
	//			ImGui::CloseCurrentPopup();
	//			ImGui::EndPopup();
	//			return false;
	//
	//		}
	//		ImGui::EndPopup();
	//	}
	//	return true;
	//}


}

