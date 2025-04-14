#pragma once


#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "particle.hpp"

namespace benjamin {

    class NbodyRunner {
    private:
        int deviceId;
        cudaDeviceProp* props;

        float* d_out;

        size_t N;
        size_t size;

        dim3 threadsPerBlock;
        dim3 blocksPerGrid;
  
        cudaError_t step_error;
        cudaError_t async_error;

    public:
        NbodyRunner(float* in, unsigned int N);
        ~NbodyRunner();
        void doNbody(float timeStep);
        void collectResults(float* destination);

    };


}
;