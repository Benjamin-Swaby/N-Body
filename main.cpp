#include <iostream>
#include <thread>
#include <random>



#include "window.hpp"
#include "particle.hpp"

#include "kernel.cuh"



struct IDs { unsigned int VAO; unsigned int VBO; unsigned int EBO; };


void RectangleAtPoint(IDs buffers, unsigned int shaderProgram, float x, float y, float size) {


	auto VAO = buffers.VAO;
	auto VBO = buffers.VBO;
	auto EBO = buffers.EBO;


	const float vertices[] = {
		x,			y,			0.0f, //BL
		x + size,	y,			0.0f, //BR
		x,			y + size,	0.0f, //TL
		x + size,	y + size,	0.0f  //TR
	};


	const int indicies[] = {
		3,1,2,
		1,0,2
	};


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);



	// ---  Draw ---- // 
	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	
}



void drawParticles(float* particles, unsigned int sp, IDs buffers, int N) {

	for (int i = 0; i < N; i += 8) {
		benjamin::punion* p = (benjamin::punion*)(&particles[i]);
		
		RectangleAtPoint(buffers, sp, p->x, p->y, p->mass);


	}


}



void initFloat(float* arr, size_t n) {

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.003, 0.004);
	std::uniform_real_distribution<float> dis2(-1, 1);


	for (int i = 0; i < n; i+=8) {
		arr[i] = dis2(gen);
		arr[i + 1] = dis2(gen);
		arr[i + 2] = 0.0f;
		arr[i + 3] = (i % 2) ? 0.0004f : -0.0004f;
		arr[i + 4] = 0.0f;
		arr[i + 5] = 0.0f;
		arr[i + 6] = dis(gen);
		arr[i + 7] = 0.0f;
	}
}


void stdoutParticles(float* p, size_t n) {
	for (int i = 0; i < n; i+=8) {
		std::cout << i / 8 << ": \t";
		std::cout << p[i] << ',';
		std::cout << p[i+1] << ',';
		std::cout << p[i+2] << '\t';

		std::cout << p[i+3] << ',';
		std::cout << p[i+4] << ',';
		std::cout << p[i+5] << '\t';

		std::cout << p[i+6] << ',';
		std::cout << p[i+7];
		
		std::cout << '\n';
	}
}

int main() {

#define N 8192
	
	float* myParticlesFP = (float*)malloc(sizeof(float) * N);

	benjamin::NbodyRunner* nb;
	
	auto win = new benjamin::Window(1080, "N-Body");
	IDs bufferIds;

	win->addSetupFunction({ "Compile Shader Program", [&win]() {
			win->compileParticleShaderProgram();
			return 0;
		} });

	win->addSetupFunction({ "Init Particles", [myParticlesFP]() {
		initFloat(myParticlesFP, N);
		//stdoutParticles(myParticlesFP, N);
		return 0;
		} });

	win->addSetupFunction({ "Init Device", [myParticlesFP, &nb]() {
			nb = new benjamin::NbodyRunner(myParticlesFP, N);
			return 0;
		} });


	float time = 0;

	win->addLoopFunction({ "Step", [&nb, myParticlesFP, &time]() {
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			nb->doNbody(0.000001f);
			nb->collectResults(myParticlesFP);
			//time += 0.5f;
			//std::cout << "----- Time = " << time << '\n';
			//stdoutParticles(myParticlesFP, N);
			//std::cout << '\n';
		} });


	win->addLoopFunction({ "Test", [&bufferIds, &win, &myParticlesFP]() {
		drawParticles(myParticlesFP, win->particleShaderProgram, bufferIds, N);
		} });



	win->Start();


	return 0;



	// TODO: Render a benjamin::particle as a square proportional in size to it's mass
	// TODO: Write CUDA Kernel for: N-Body Step, + Collision Detection (Assume Elastic).
	// TODO: Render each step inturn. (CUDA, OPENGL Can share buffers)

}
    

