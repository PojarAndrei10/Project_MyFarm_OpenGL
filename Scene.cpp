#include "Scene.h"
namespace gps {
    GLFWwindow* Scene::getScene()
    {
        return this->scene;
    }
    DIM Scene::getSceneDimensions()
    {
        return this->dim;
    }
    void Scene::setSceneDimensions(DIM dim)
    {
        this->dim = dim;
    }
    void Scene::create(const char* ch,int a, int b)
    {
        if (!glfwInit())
        {
            throw std::runtime_error("Could not start !");

        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

        glfwWindowHint(GLFW_SAMPLES, 4);

        this->scene = glfwCreateWindow(a, b, ch, NULL, NULL);
        if (!this->scene) 
        {
            throw std::runtime_error("Could not create GLFW3 window");
        }
        glfwMakeContextCurrent(scene);

        glfwSwapInterval(1);
        glewExperimental = GL_TRUE;
        glewInit();
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version = glGetString(GL_VERSION); 
        std::cout << "Renderer OpenGL:" << renderer << std::endl;
        std::cout << "Version:" << version << std::endl;

        //for RETINA display
        glfwGetFramebufferSize(scene, &this->dim.a, &this->dim.b);
    }
    void Scene::Delete() 
    {
        if (scene)
        {
            glfwDestroyWindow(scene);
        }
        glfwTerminate();
    }
    

    

}