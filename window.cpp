#include "window.hpp"


#include <iostream>
#include <fstream>
#include <sstream>


benjamin::Window::Window(int size, std::string name) {
	
	using	std::string, 
			std::cout,
			std::cerr,
			std::endl;
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	this->obj = glfwCreateWindow(size, size, name.c_str(), NULL, NULL);

	if (!this->obj) {
		glfwTerminate();
		cerr << "Failed To Create Window" << endl;
		exit(1);
	}

	glfwMakeContextCurrent(this->obj);
	glfwSwapInterval(1);
	gladLoadGL(glfwGetProcAddress);

	glfwSetWindowSizeLimits(this->obj, size, size, GLFW_DONT_CARE, GLFW_DONT_CARE);

	// lock aspect Ratio
	glfwSetWindowAspectRatio(this->obj, 1, 1);


	glfwGetFramebufferSize(this->obj, &this->size, &this->size);
	glViewport(0, 0, this->size, this->size);



}





void benjamin::Window::RenderExecuteLoop() {

	while (!glfwWindowShouldClose(this->obj)) {
		
		glfwGetFramebufferSize(this->obj, &this->size, &this->size);
		glViewport(0, 0, this->size, this->size);
		glClearColor(0.25, 0.3, 0.3, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		


	


		for (const auto& stage : this->loop) {
			stage.func();
		}


		glfwSwapBuffers(this->obj);
		glfwPollEvents();

	}


}


void benjamin::Window::Start() {
	
	std::cout << "Setup:\n";
	for (auto& stage : this->setup) {
		std::cout << '\t' << stage.label << ": ";
		std::cout << stage.func() << '\n';
	}
	
	this->RenderExecuteLoop();
}


benjamin::Window::~Window() {

	glfwTerminate();
}

void benjamin::Window::addSetupFunction(benjamin::job<int> func) {
	this->setup.push_back(func);
}

void benjamin::Window::addLoopFunction(benjamin::job<void> func) {
	this->loop.push_back(func);
}




void benjamin::Window::compileParticleShaderProgram() {
	std::ifstream vertex_source("./particle_shader.vert");

	if (!vertex_source) {
		std::cerr << "Failed To Open Particle Vertex Shader";
		return;
	}

	std::stringstream buf;
	buf << vertex_source.rdbuf();

	std::string vertexShaderSrc = buf.str();
	const GLchar* vertexSourcePtr = vertexShaderSrc.c_str();

	vertex_source.close();

	std::ifstream fragment_source("./particle_shader.frag");
	if (!fragment_source) {
		std::cerr << "Failed To Open Particle Fragment Shader";
		return;
	}


	buf.str("");
	buf.clear();
	buf.flush();

	buf << fragment_source.rdbuf();
	std::string fragmentShaderSrc = buf.str();
	const GLchar* fragmentSourcePtr = fragmentShaderSrc.c_str();


	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSourcePtr, NULL);
	glCompileShader(vertexShader);

	int success;
	char info[512];

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, info);
		std::cerr << "Failed to Compile Vertex Shader (./particle_shader.vert)\n" << info;
		std::cerr << "\nVertex Source:\n\n" << vertexShaderSrc;
		return;
	}

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSourcePtr, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, info);
		std::cerr << "Failed to Compile Fragment Shader (./particle_shader.frag)\n" << info;
		std::cerr << "\nFragment Source:\n\n" << fragmentShaderSrc;
		return;
	}


	// Link Shaders

	this->particleShaderProgram = glCreateProgram();
	glAttachShader(this->particleShaderProgram, vertexShader);
	glAttachShader(this->particleShaderProgram, fragmentShader);
	glLinkProgram(this->particleShaderProgram);
	glGetProgramiv(this->particleShaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		glGetProgramInfoLog(this->particleShaderProgram, 512, NULL, info);
		std::cerr << "Failed to Link Shaders\n" << info;
		return;
	}


	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}


