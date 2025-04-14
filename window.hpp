#pragma once

#include "job.hpp"


#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>


#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



#include <string>
#include <functional>
#include <vector>
#include <thread>

namespace benjamin {
	
	
	class Window {
	private:
		int size; // Window Dimensions
		std::string name; // Window Title
		GLFWwindow* obj; // GLFW window Object.
		
		std::vector<benjamin::job<int>> setup; // list of functions to be called during setup
		std::vector<benjamin::job<void>> loop; // list of functions to be called during the main loop

		std::thread t; // Thread containing window operation.

		

		void RenderExecuteLoop();

	public:
		unsigned int particleShaderProgram;
		Window(int size, std::string name);
		~Window();
		
		void Start();
		
		void addSetupFunction(benjamin::job<int> func);
		void addLoopFunction(benjamin::job<void> func);

		void compileParticleShaderProgram();
	};

	





	



}







