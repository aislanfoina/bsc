#define CUDA_SAFE_CALL_NO_SYNC( call) do {                                 \
        cudaError err = call;                                                    \
        if( cudaSuccess != err) {                                                \
                fprintf(stderr, "Cuda error in file '%s' in line %i : %s.\n",        \
                                __FILE__, __LINE__, cudaGetErrorString( err) );              \
                exit(EXIT_FAILURE);                                                  \
        } } while (0)

#define cutilSafeCall( call) do {                                         \
        CUDA_SAFE_CALL_NO_SYNC(call);                                            \
        cudaError err = cudaThreadSynchronize();                                 \
        if( cudaSuccess != err) {                                                \
                fprintf(stderr, "Cuda error in file '%s' in line %i : %s.\n",        \
                                __FILE__, __LINE__, cudaGetErrorString( err) );              \
                exit(EXIT_FAILURE);                                                  \
        } } while (0)


