#include "kernel.cuh"

#define BIG_G 6.67-11



namespace benjamin {



    __device__ inline benjamin::v3_t cross_product(v3_t a, v3_t b) {
        v3_t output;
        output.x = a.y * b.z - a.z * b.y;
        output.y = a.z * b.x - a.x * b.z;
        output.z = a.x * b.y - a.y * b.x;
        return output;
    }


    __device__ inline double dot_product(v3_t a, v3_t b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    __device__ inline double magnitude(v3_t a) {
        return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
    }

    __device__ inline v3_t subtract(v3_t a, v3_t b) {
        v3_t output;
        output.x = a.x - b.x;
        output.y = a.y - b.y;
        output.z = a.z - b.z;
        return output;
    }

    __device__ double min_distance(v3_t p1, v3_t p2, v3_t q1, v3_t q2) {
        
        v3_t d1 = subtract(p2, p1);
        v3_t d2 = subtract(q2, q2);
        v3_t r = subtract(q1, p1);
        v3_t cross = cross_product(d1, d2);
        double cross_mag = magnitude(cross);


        if (cross_mag == 0) {
            v3_t r_cross_d1 = cross_product(r, d1);
            return magnitude(r_cross_d1) / magnitude(d1);
        }

        return fabs(dot_product(r, cross) / cross_mag);
        
    }

    /**
        Collide:
        This will perform simple predictive elastic collision between particles

    */
    __global__ void collide(float* out, size_t N, float timeStep) {
    

        int index = blockIdx.y * blockDim.y + (threadIdx.y * 8);
        int stride = gridDim.y * (blockDim.y * 8);
        
        for (int i = index; i < N; i+= stride) {
            
            for (int target = 0; target < N; target += 8) {
                if (target == i)
                    continue;


                // check if out[i]'s movement vector (x->x+dx, y->y+dx, z->z+dx) intersects 
                // with in mass of target(x->x+dx, y->y+dx z->z+dx) 

                benjamin::v3_t Start = { out[i], out[i + 1], out[i + 2] };
                benjamin::v3_t End = { out[i] + out[i + 3], out[i + 1] + out[i + 4], out[i + 2] + out[i + 5] };
                benjamin::v3_t targetStart = { out[target], out[target + 1], out[target + 2] };
                benjamin::v3_t targetEnd = { out[target] + out[target + 3], out[target + 1] + out[target + 4], out[target + 2] + out[target + 5] };


                double distance = min_distance(Start, End, targetStart, targetEnd);

                // Collision occurs
                if (distance <= (out[i + 6] + out[target + 6]) / 2) {
                    out[i + 3] *= -1;
                    out[i + 4] *= -1;
                    out[i + 5] *= -1;
                    //printf("C: %d x %d = %f\n", i/8, target/8, distance );
                }

            }


        }

    }


    /**
        move:
        this will apply acceleration on the output. 
    */
    __global__ void move(float* out, size_t N) {
        
        int index = blockIdx.y * blockDim.y + (threadIdx.y * 8);
        int stride = gridDim.y * (blockDim.y * 8);

        for (int i = index; i < N; i += stride) {
            out[i]   += out[i+3];
            out[i+1] += out[i+4];
            out[i+2] += out[i+5];
            out[i + 7]++;
        }

    }
    

    /**
        Calculate Next Projection:
        Args : Input State, output state, size

        This will apply Newton's Gravitational Behavoir to particles.
        
    */
    __global__ void calculate_next_projection(float* out,size_t N, double time_step) {
 
        int index = blockIdx.y * blockDim.y + (threadIdx.y * 8);
        int stride = gridDim.y * (blockDim.y * 8);

        for (int i = index; i < N; i += stride) {

            benjamin::v3_t a = { 0.0f, 0.0f, 0.0f };


            // for every other particle 
            for (int target = 0; target < N; target+=8) {
                if (target == i)
                    continue;

                benjamin::v3_t deltaPosition = {
                    out[i] - out[target],
                    out[i+1] - out[target+1],
                    out[i+2] - out[target+2],
                };

                float distance = sqrtf(
                    deltaPosition.x * deltaPosition.x +
                    deltaPosition.y * deltaPosition.y +
                    deltaPosition.z * deltaPosition.z
                );

                float accel = -1 * BIG_G * (out[target+6]) / powf(distance, 2.0f);

                benjamin::v3_t ruv = {
                    deltaPosition.x / distance,
                    deltaPosition.y / distance,
                    deltaPosition.z / distance 
                };

                a.x += ruv.x * accel / 10000;
                a.y += ruv.y * accel / 10000;
                a.z += ruv.z * accel / 10000;

                out[i+3] += a.x * time_step;
                out[i+4] += a.y * time_step;
                out[i+5] += a.z * time_step;

            }


        }

    }


    cudaDeviceProp getDetails(int deviceId) {
        cudaDeviceProp props;
        cudaGetDeviceProperties(&props, deviceId);
        return props;
    }


    NbodyRunner::NbodyRunner(float* in, unsigned int N) {
        
        cudaGetDevice(&this->deviceId);
  
        this->N = N;
        this->size = N * sizeof(float);
       
        cudaMalloc((void**)&this->d_out, size);
        cudaMemcpy(this->d_out, in, size, cudaMemcpyHostToDevice);
    
        unsigned int elementsPerBlock = 256;
        this->threadsPerBlock = dim3{1,elementsPerBlock,1}; // Number of threads per block
        this->blocksPerGrid = dim3{ 1, (N/elementsPerBlock), 1 };
 
    }

    void NbodyRunner::doNbody(float timeStep) {
        

        calculate_next_projection<<<blocksPerGrid, threadsPerBlock>>>(this->d_out, this->N, timeStep);  
        move<<<blocksPerGrid, threadsPerBlock >>>(this->d_out, this->N);
        collide <<<blocksPerGrid, threadsPerBlock >>> (this->d_out, this->N, timeStep);

    }


    NbodyRunner::~NbodyRunner() {
        cudaFree(this->d_out);
    }

    void NbodyRunner::collectResults(float* destination) {

        cudaDeviceSynchronize();
        cudaMemcpy(destination, this->d_out, this->size, cudaMemcpyDeviceToHost);
        
 

    }
    

}

