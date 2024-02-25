#pragma once
#ifndef SCENE_H
#define SCENE_H
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct DIM
{
	int a; //width=latime
	int b; //height=inaltime
};
namespace gps {
	class Scene
	{
	public:
		void create(const char* title = "PG",
			int a = 1000, int b = 800);
		void Delete();
		void setSceneDimensions(DIM dim);
		DIM getSceneDimensions();
		GLFWwindow* getScene();
		GLFWwindow* scene;
		DIM dim;
		
	};
}
#endif