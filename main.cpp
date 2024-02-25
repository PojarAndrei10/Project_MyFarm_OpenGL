#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.hpp"
#include "SkyBox.hpp"
#include "Scene.h"
#include "Shader.hpp"
#include "Model3D.hpp"
gps::Scene mySceneFromBlender;

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <chrono>
#include <windows.h>
#include <stdio.h>

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::mat4 lightRotation;
glm::mat4 balloonModel;
glm::mat4 sunModel;
glm::mat4 droneModel;

// celelalte GLuint-uri
GLuint lightModeLoc;
GLuint hazeLoc;
GLuint hazeViewLoc;
GLuint cameraPosLoc;

gps::Camera myCamera(
	glm::vec3(38.0f, 60.0f, 19.3f),
	glm::vec3(0.0f, 1.0f, -8.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.01f;

GLboolean pressedKeys[1024];
GLfloat angle;
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D sun;
gps::Model3D quad;
gps::Model3D sceneBlender;
gps::Model3D balloon;
gps::Model3D drone;
gps::Model3D lightCube;

gps::Shader lightShader;
gps::Shader skyBoxShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader myCustomShader;

gps::SkyBox mySkyBox;
gps::SkyBox day;
gps::SkyBox night;

FILE* file;

//alte variabile aditionale
int seeTheLight = 0;
bool presentationModeOn = false;
bool showDepthMap = false;
bool ok = false;  

float viewHaze;
float balloonSpeed = 0.005;
float okPresentation1 = 0,okPresentation2=0,okPresentation3=0;
float droneRotation = 0.06f;

GLuint shadowMapFBO;
GLuint depthMapTexture;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

std::vector<const GLchar*> dayy;
std::vector<const GLchar*> dark;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	static double lastX = 0.0;
	static double lastY = 0.0;
	if (lastX == 0.0) 
	{
		lastX = xpos;
		lastY = ypos;
	}

	double xOffset = xpos - lastX;
	double yOffset = lastY - ypos;
	const float sensitivity = 0.0008f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;
	myCamera.rotate((float)yOffset, (float)xOffset);

	lastX = xpos;
	lastY = ypos;
}
void updateViewAndNormalMatrix()
{
	myCustomShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}
void keyPress(const char* key)
{
	if (ok)
		fprintf(file, "%s\n", key);
}
void presentationMode() {
	myCustomShader.useShaderProgram();
	glm::vec3 centerOfRotation = glm::vec3(12.81f, 10.33f, -10.56f);
	float radius = 50.0f;
	float angleRadians = glm::radians(okPresentation1);

	//calculeaza noile coordonate ale camerei in jurul centrului de rotatie
	float cameraX = centerOfRotation.x + radius * glm::sin(angleRadians);
	float cameraZ = centerOfRotation.z + radius * glm::cos(angleRadians);

	//seteaza noua pozitie a camerei
	myCamera.cameraPosition.x = cameraX;
	myCamera.cameraPosition.z = cameraZ;

	//seteaza punctul catre care camera este orientata
	myCamera.cameraTarget = centerOfRotation;

	// Actualizeaza matricea de vizualizare
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// Actualizeaza matricea normala
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	// Creste unghiul pentru urmatorul cadru
	okPresentation1 += 0.5f;
	presentationModeOn = true;
}
void processMovement()
{
	//tastele
	myCustomShader.useShaderProgram();
	if (pressedKeys[GLFW_KEY_P]) {
		presentationMode();
	}
	if (pressedKeys[GLFW_KEY_W])
	{
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed * 0.1);
		updateViewAndNormalMatrix();
		keyPress("W");
	}
	if (pressedKeys[GLFW_KEY_S])
	{
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed * 0.1);
		updateViewAndNormalMatrix();
		keyPress("S");
	}
	if (pressedKeys[GLFW_KEY_A])
	{
		myCamera.move(gps::MOVE_LEFT, cameraSpeed * 0.1);
		updateViewAndNormalMatrix();
		keyPress("A");
	}
	if (pressedKeys[GLFW_KEY_D])
	{
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed * 0.1);
		updateViewAndNormalMatrix();
		keyPress("D");
	}
	if (pressedKeys[GLFW_KEY_UP])
	{
		myCamera.rotate(0.009f, 0.0f);
		updateViewAndNormalMatrix();
		keyPress("UP");
	}
	if (pressedKeys[GLFW_KEY_DOWN])
	{
		myCamera.rotate(-0.009f, 0.0f);
		updateViewAndNormalMatrix();
		keyPress("DOWN");
	}
	if (pressedKeys[GLFW_KEY_RIGHT])
	{
		myCamera.rotate(0.0f, -0.02f);
		updateViewAndNormalMatrix();
		keyPress("RIGHT");
	}
	if (pressedKeys[GLFW_KEY_LEFT])
	{
		myCamera.rotate(0.0f, 0.02f);
		updateViewAndNormalMatrix();
		keyPress("LEFT");
	}
	if (pressedKeys[GLFW_KEY_Q])
	{
		myCustomShader.useShaderProgram();
		angle = angle + 1.0f;
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		balloonModel = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}
	if (pressedKeys[GLFW_KEY_E])
	{
		myCustomShader.useShaderProgram();
		angle = angle - 1.0f;
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		balloonModel = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

}
void modes() 
{   //vizualizare scena in modurile: solid->5,wireframe->6,points->7
	if (pressedKeys[GLFW_KEY_5]) 
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (pressedKeys[GLFW_KEY_6]) 
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (pressedKeys[GLFW_KEY_7]) 
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
}
void haze() 
{
	//pornire,oprire ceata,marirea si miscorarea densitatii a cetei
	myCustomShader.useShaderProgram();
	if (pressedKeys[GLFW_KEY_Z])
		glUniform1i(hazeLoc, 1);
	else if(pressedKeys[GLFW_KEY_X])
		glUniform1i(hazeLoc, 0);
	else if (pressedKeys[GLFW_KEY_C])
	{
		if (viewHaze < 1)
		{
			viewHaze =viewHaze+ 0.0003;
			glUniform1f(hazeViewLoc, viewHaze);

		}
	}
	else if (pressedKeys[GLFW_KEY_V])
	{
		if (viewHaze >0)
		{
			viewHaze =viewHaze- 0.0003;
			glUniform1f(hazeViewLoc, viewHaze);
		}
	}
}
void lightControl() 
{
	//tastele pt lumina
	myCustomShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	if (pressedKeys[GLFW_KEY_J] || pressedKeys[GLFW_KEY_L])
	{
		lightAngle += (pressedKeys[GLFW_KEY_J] ? -1.0f : 1.0f);
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation))
			* lightDir));
	}
	if (pressedKeys[GLFW_KEY_R])
		seeTheLight = 0;
	else if (pressedKeys[GLFW_KEY_Y])
		seeTheLight = 1;
	glUniform1i(lightModeLoc, seeTheLight);
}
void initOpenGLWindow() {
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return;
	}
	mySceneFromBlender.create("My Project->Pojar Andrei-Gabriel",2000, 1200);
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(mySceneFromBlender.getScene(), windowResizeCallback);
	glfwSetKeyCallback(mySceneFromBlender.getScene(), keyboardCallback);
	glfwSetCursorPosCallback(mySceneFromBlender.getScene(), mouseCallback);
}
void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3F, 1.0f);
	glViewport(0, 0, mySceneFromBlender.getSceneDimensions().a, mySceneFromBlender.getSceneDimensions().b);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}
void initObjects() 
{
	//Incarc scena+obiectele individuale
	sceneBlender.LoadModel("models/Scene/scenaaaaaaaa.obj");
	balloon.LoadModel("models/Balon/balon.obj");
	sun.LoadModel("models/sun/sun.obj");
	quad.LoadModel("objects/quad/quad.obj");
	drone.LoadModel("models/Drone/dronee.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
}
void initShaders() 
{
	myCustomShader.loadShader("shaders/shaderStart.vert","shaders/shaderStart.frag");
	lightShader.loadShader("shaders/lightCube.vert","shaders/lightCube.frag");
	skyBoxShader.loadShader("shaders/skyboxShader.vert","shaders/skyboxShader.frag");
	screenQuadShader.loadShader("shaders/screenQuad.vert","shaders/screenQuad.frag");
	depthMapShader.loadShader("shaders/depthMap.vert","shaders/depthMap.frag");
}
void initUniforms() {
	myCustomShader.useShaderProgram();
	balloonModel = glm::mat4(1.0f);
	droneModel = glm::mat4(1.0f);
	sunModel = glm::mat4(1.0f);

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	
	projection = glm::perspective(glm::radians(45.0f),
		(float)mySceneFromBlender.getSceneDimensions().a / (float)mySceneFromBlender.getSceneDimensions().b,
		0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");

	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(28.0f, 68.0f, -25.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	glUseProgram(myCustomShader.shaderProgram);

	int lanternPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lanternPosition");
	int lanternColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lanternColor");
	glm::vec3 lanternPosition(1.0f, 100.0f, 1.0f);
	glm::vec3 lanternColor(1.0f, 1.0f, 0.0f);  
	glUniform3fv(lanternPosLoc, 1, glm::value_ptr(lanternPosition));
	glUniform3fv(lanternColorLoc, 1, glm::value_ptr(lanternColor));

	hazeLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogEnable");
	glUniform1i(hazeLoc, 0);
	hazeViewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
	glUniform1f(hazeViewLoc, 0.0035);

	lightModeLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightMode");
	glUniform1i(lightModeLoc, 0);
	cameraPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "cameraPos");
	glUniform3fv(cameraPosLoc, 1, glm::value_ptr(myCamera.cameraPosition));
	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	skyBoxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	projection = glm::perspective(glm::radians(45.0f), 1800.0f / 1080, 0.1f, 800.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "view"), 1, GL_FALSE,glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "projection"), 1, GL_FALSE,glm::value_ptr(projection));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate fbo id
	glGenFramebuffers(1, &shadowMapFBO);
	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	//lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 10.0f;
	//const GLfloat near_plane = 0.1f, far_plane = 80.0f;
	//const GLfloat near_plane = -80.0f, far_plane = 90.0f
	//glm::mat4 lightProjection = glm::ortho(-40.0f, 90.0f, -40.0f, 40.0f, near_plane, far_plane);
	glm::mat4 lightProjection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}
void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
}
void setupShader(gps::Shader& shader)
{
	shader.useShaderProgram();
}
void setModel()
{
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
}
void calculateAndSetNormalMatrix()
{
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}
void modellingScene(bool o,gps::Shader& shadere)
{
	setupShader(myCustomShader);
	setModel();
	if (!o)
		calculateAndSetNormalMatrix();
	sceneBlender.Draw(myCustomShader);
}
void renderDrone(gps::Shader shader, bool verif) 
{
	setupShader(shader);
	glm::mat4 originalModel = droneModel;
	float translationDistance = sin(glfwGetTime()) * 5.0f;
	droneModel = glm::translate(originalModel, glm::vec3(0.0f, 3.16298f + translationDistance, 5.0f));

	droneModel = glm::rotate(droneModel, droneRotation, glm::vec3(0.0f, 1.0f, 0.0f));

	float rotationAngle = glfwGetTime();
	droneModel = glm::rotate(droneModel, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	droneModel = glm::translate(droneModel, glm::vec3(0.0f, -3.16298f, -5.0f));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(droneModel));
	if (!verif)
	{
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	drone.Draw(shader);
	droneModel = originalModel;
}
void renderBalloon(gps::Shader shader, bool verif) {
	//fac balonul sa se miste
	shader.useShaderProgram();
	glm::mat4 originalModel = balloonModel;
	float translationDistance = sin(glfwGetTime()) * 5.0f;
	balloonModel = glm::translate(originalModel, glm::vec3(-7.0, 8.0, -45.0 + translationDistance));

	balloonModel = glm::rotate(balloonModel, balloonSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
	balloonModel = glm::translate(balloonModel, glm::vec3(7.0, -8.0, 45.0));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(balloonModel));
	if (!verif) 
	{
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	balloon.Draw(shader);
	balloonModel = originalModel;
}
void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map
	//render the scene to the depth buffer
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	modellingScene(true, myCustomShader);
	renderBalloon(myCustomShader, true);
	renderDrone(myCustomShader, true);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, mySceneFromBlender.getSceneDimensions().a,
			mySceneFromBlender.getSceneDimensions().b);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		quad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, mySceneFromBlender.getSceneDimensions().a,
			mySceneFromBlender.getSceneDimensions().b);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		modellingScene(false, myCustomShader);
		renderBalloon(myCustomShader, false);
		renderDrone(myCustomShader, false);
		setupShader(skyBoxShader);
		if (seeTheLight == 0)
			day.Draw(skyBoxShader, view, projection);
		else night.Draw(skyBoxShader, view, projection);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}
void cleanup() 
{
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	//glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}
int presentationState = 0;
void tasteP() {
	//prezentare cand dai click pe tasta P
	cameraSpeed = 0.1f;
	if (presentationModeOn) 
	{
		cameraSpeed = 0.01f;
		switch (presentationState) {
		case 0:
			myCamera.rotate(0.0f, 0.0065f);
			okPresentation1 += 0.45;
			if (okPresentation1 >= 180) {
				presentationState++;
			}
			break;
		case 1:
			myCamera.rotate(0.0f, 0.0065f);
			okPresentation2 += 0.45;
			if (okPresentation2 >99) {
				presentationState++;
			}
			break;
		case 2:
			if (myCamera.cameraTarget.y<=-2) {
				myCamera.rotate(0.003f, 0.0f);
			}
			else {
				presentationState++;
			}
			break;
		case 3:
			cameraSpeed = 0.045f;
			myCamera.rotate(0.0f, 0.0075f);
			okPresentation3 += 0.45;
			if (okPresentation3 >= 180) {
				presentationState++;
			}
			break;
		case 4:
			cameraSpeed = 0.045f;
			myCamera.move(gps::MOVE_UP, cameraSpeed);
			if (myCamera.cameraPosition.y >= 26) {
				presentationState++;
			}
			break;
		case 5:
			myCamera.rotate(-0.006f, 0.0f);
			if (myCamera.cameraTarget.y <= 10) {
				presentationModeOn = false;
			}
			break;
		default:
			break;
		}
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}
	else {
		cameraSpeed = 0.1f;
		okPresentation1 = 0;okPresentation2 = 0;okPresentation3 = 0;
		presentationState = 0;
	}
}
void initSkybox() 
{
	dayy.push_back("skybox/right.tga");
	dayy.push_back("skybox/left.tga");
	dayy.push_back("skybox/top.tga");
	dayy.push_back("skybox/bottom.tga");
	dayy.push_back("skybox/back.tga");
	dayy.push_back("skybox/front.tga");
	dark.push_back("skybox/top.tga");
	day.Load(dayy);
	night.Load(dark);
}

int main(int argc, const char* argv[]) {

	try {
		initOpenGLWindow();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	setWindowCallbacks();
	glCheckError();
	initSkybox();
	
	while (!glfwWindowShouldClose(mySceneFromBlender.getScene()))
	{
		if (!presentationModeOn)
			processMovement();
		modes();
		haze();
		lightControl();
		renderScene();
		tasteP();
		glfwPollEvents();
		glfwSwapBuffers(mySceneFromBlender.getScene());
		glCheckError();
	}
	cleanup();
	return 0 ;
}
