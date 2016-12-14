#ifndef SHADOW_KERNEL_HPP
#define SHADOW_KERNEL_HPP

#if defined(USE_CUDA)
#include "cublas_v2.h"
#include "cuda_runtime.h"

// CUDA: use 512 threads per block
const int NumThreads = 512;

// CUDA: number of blocks for threads
inline int GetBlocks(const int N) { return (N + NumThreads - 1) / NumThreads; }

// CUDA: grid stride looping
#define CUDA_KERNEL_LOOP(i, n)                                 \
  for (int i = blockIdx.x * blockDim.x + threadIdx.x; i < (n); \
       i += blockDim.x * gridDim.x)

#define CheckError(status)                                                \
  {                                                                       \
    if (status != cudaSuccess) {                                          \
      LOG(FATAL) << "Error: " << std::string(cudaGetErrorString(status)); \
    }                                                                     \
  }

#elif defined(USE_CL)
#include "CLCudaAPI/clpp11.h"
#endif

namespace Kernel {

void Setup(int device_id = 0);
void Release();

template <typename T, typename Dtype>
T *MakeBuffer(int size, Dtype *host_ptr);

template <typename T, typename Dtype>
void ReadBuffer(int size, const T *src, Dtype *des);

template <typename T, typename Dtype>
void WriteBuffer(int size, const Dtype *src, T *des);

template <typename T, typename Dtype>
void CopyBuffer(int size, const T *src, T *des);

template <typename T>
void ReleaseBuffer(T *buffer);

#if defined(USE_CUDA)
extern cublasHandle_t cublas_handle_;

#elif defined(USE_CL)
extern CLCudaAPI::Context *context_;
extern CLCudaAPI::Queue *queue_;
extern CLCudaAPI::Event *event_;

extern CLCudaAPI::Kernel *cl_channelmax_kernel_;
extern CLCudaAPI::Kernel *cl_channelsub_kernel_;
extern CLCudaAPI::Kernel *cl_channelsum_kernel_;
extern CLCudaAPI::Kernel *cl_channeldiv_kernel_;
extern CLCudaAPI::Kernel *cl_set_kernel_;
extern CLCudaAPI::Kernel *cl_add_kernel_;
extern CLCudaAPI::Kernel *cl_sub_kernel_;
extern CLCudaAPI::Kernel *cl_mul_kernel_;
extern CLCudaAPI::Kernel *cl_div_kernel_;
extern CLCudaAPI::Kernel *cl_sqr_kernel_;
extern CLCudaAPI::Kernel *cl_exp_kernel_;
extern CLCudaAPI::Kernel *cl_log_kernel_;
extern CLCudaAPI::Kernel *cl_abs_kernel_;
extern CLCudaAPI::Kernel *cl_pow_kernel_;

extern CLCudaAPI::Kernel *cl_datatransform_kernel_;
extern CLCudaAPI::Kernel *cl_im2col_kernel_;
extern CLCudaAPI::Kernel *cl_pooling_kernel_;
extern CLCudaAPI::Kernel *cl_concat_kernel_;
extern CLCudaAPI::Kernel *cl_permute_kernel_;
extern CLCudaAPI::Kernel *cl_activate_kernel_;
#endif

}  // namespace Kernel

#endif  // SHADOW_KERNEL_HPP
