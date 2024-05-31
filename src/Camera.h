#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"

class Camera
{
private:
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	float cameraFov;

	bool firstMouse;

	float lastX;
	float lastY;
	float yaw;
	float pitch;

public:
	Camera();
	void MouseControl(double xposIn, double yposIn);
	void ScrollControl(double xoffset, double yoffset);
	void KeyControl(GLFWwindow* window,const float deltaTime);
	glm::mat4 SetView();
	glm::mat4 SetProjection(float aspect);
	inline glm::vec3 GetPosition() const { return cameraPos; }
	inline void SetFirstMouse() { firstMouse = true; }
	void SetCamera(glm::vec3 pos, glm::vec3 front, glm::vec3 up, float fov, float yaw, float pitch);
};
