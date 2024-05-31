#include "Camera.h"
#include <iostream>

Camera::Camera() :
	cameraPos(glm::vec3(40.0f, 0.0f, 0.0f)), cameraFront(glm::vec3(-1.0f, 0.0f, 0.0f)),
	cameraUp(glm::vec3(0.0f, 1.0f, 0.0f)), cameraFov(10.0f), firstMouse(true),
	lastX(0), lastY(0), yaw(180.0f), pitch(0.0f)
{
}
	
	

void Camera::MouseControl(double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);//获取当前坐标
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)//如果是第一下移动鼠标，需避免视角突变
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;//鼠标位移量
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	yaw += xoffset * 0.1f;//水平旋转角
	pitch += yoffset * 0.1f;//俯仰角

	//std::cout << yaw << std::endl;
	//std::cout << pitch << std::endl;

	
	if (pitch > 89.0f)//限制俯仰角度
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));//换算为照相机前向的矢量分量
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}
void Camera::ScrollControl(double xoffset, double yoffset)
{
	cameraFov -= 0.4f*(float)yoffset;
	if (cameraFov < 0.0f)
		cameraFov = 0.0f;
	if (cameraFov > 120.0f)
		cameraFov = 120.0f;
}

void Camera::KeyControl(GLFWwindow* window,const float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = static_cast<float>(1.0f * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

glm::mat4 Camera::SetView()
{
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	return view;
}
glm::mat4 Camera::SetProjection(float aspect)
{
	glm::mat4 projection = glm::perspective(glm::radians(cameraFov), aspect, 0.001f, 10000.0f);
	return projection;
}


void Camera::SetCamera(glm::vec3 pos,glm::vec3 front, glm::vec3 up, float fov, float y, float p)
{
	cameraPos = pos;
	cameraFront = front;
	cameraUp = up;
	cameraFov = fov;
	firstMouse = true;
	yaw = y;
	pitch = p;
}
