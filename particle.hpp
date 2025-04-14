#pragma once
#include <cstdint>
#include <iostream>
#include <glad/gl.h>


#include "cuda_runtime.h"
#include "device_launch_parameters.h"

namespace benjamin {

	
	typedef struct __align__(16) v3 {
		float x; float y; float z;
	} v3_t;


	typedef struct {
		float x, y, z;
		float dx, dy, dz;
		float mass;
	} particle_t;

	union __align__(1) punion {
		struct {
			float x, y, z;
			float dx, dy, dz;
			float mass;
			float collisions;
		};
		float data[8];
	};

	

}


