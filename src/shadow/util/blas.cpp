#include "shadow/util/blas.hpp"

void Blas::SetArray(int N, float value, BType *out_data) {
#if !defined(USE_CUDA) & !defined(USE_CL)
  std::fill(out_data, out_data + N, value);

#else
  Kernel::SetArray(N, value, out_data);
#endif
}

void Blas::SetArrayRepeat(int N, const BType *value, int value_size,
                          BType *out_data, int offset) {
#if !defined(USE_CUDA) & !defined(USE_CL)
  for (int i = 0; i < value_size; ++i) {
    BType *out_data_offset = out_data + offset + i * N;
    std::fill(out_data_offset, out_data_offset + N, value[i]);
  }

#else
  Kernel::SetArrayRepeat(N, value, value_size, out_data, offset);
#endif
}

void Blas::BlasCopy(int N, const BType *X, int incx, BType *Y, int offset,
                    int incy) {
#if defined(USE_CUDA)
  cublasScopy(reinterpret_cast<cublasHandle_t>(Kernel::GetHandle()), N, X, incx,
              Y + offset, incy);

#elif defined(USE_CL)
  clblasScopy(N, *X, 0, incx, *Y, offset, incy, 1,
              reinterpret_cast<cl_command_queue *>(Kernel::GetQueue()), 0,
              nullptr, nullptr);

#else
  for (int i = 0; i < N; ++i) {
    Y[offset + i * incy] = X[i * incx];
  }
#endif
}

void Blas::BlasAxpy(int N, float ALPHA, const float *X, int INCX, float *Y,
                    int INCY) {
  for (int i = 0; i < N; ++i) Y[i * INCY] += ALPHA * X[i * INCX];
}

void SGemmNN(int M, int N, int K, float ALPHA, const float *A, int lda,
             const float *B, int ldb, float *C, int ldc) {
  for (int i = 0; i < M; ++i) {
    for (int k = 0; k < K; ++k) {
      float A_PART = ALPHA * A[i * lda + k];
      for (int j = 0; j < N; ++j) {
        C[i * ldc + j] += A_PART * B[k * ldb + j];
      }
    }
  }
}

void SGemmNT(int M, int N, int K, float ALPHA, const float *A, int lda,
             const float *B, int ldb, float *C, int ldc) {
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; ++j) {
      float sum = 0;
      for (int k = 0; k < K; ++k) {
        sum += ALPHA * A[i * lda + k] * B[j * ldb + k];
      }
      C[i * ldc + j] += sum;
    }
  }
}

void SGemmTN(int M, int N, int K, float ALPHA, const float *A, int lda,
             const float *B, int ldb, float *C, int ldc) {
  for (int i = 0; i < M; ++i) {
    for (int k = 0; k < K; ++k) {
      float A_PART = ALPHA * A[k * lda + i];
      for (int j = 0; j < N; ++j) {
        C[i * ldc + j] += A_PART * B[k * ldb + j];
      }
    }
  }
}

void SGemmTT(int M, int N, int K, float ALPHA, const float *A, int lda,
             const float *B, int ldb, float *C, int ldc) {
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; ++j) {
      float sum = 0;
      for (int k = 0; k < K; ++k) {
        sum += ALPHA * A[i + k * lda] * B[k + j * ldb];
      }
      C[i * ldc + j] += sum;
    }
  }
}

#if defined(USE_CUDA)
void Blas::BlasSGemm(int TA, int TB, int M, int N, int K, float ALPHA,
                     const float *bufA, int lda, const float *bufB, int ldb,
                     float BETA, float *bufC, int offset, int ldc) {
  cublasOperation_t transA = TA ? CUBLAS_OP_T : CUBLAS_OP_N;
  cublasOperation_t transB = TB ? CUBLAS_OP_T : CUBLAS_OP_N;
  cublasSgemm(reinterpret_cast<cublasHandle_t>(Kernel::GetHandle()), transA,
              transB, N, M, K, &ALPHA, bufB, ldb, bufA, lda, &BETA,
              bufC + offset, ldc);
}

#elif defined(USE_CL)
void Blas::BlasSGemm(int TA, int TB, int M, int N, int K, float ALPHA,
                     const cl_mem *bufA, int lda, const cl_mem *bufB, int ldb,
                     float BETA, cl_mem *bufC, int offset, int ldc) {
  clblasTranspose transA = TA ? clblasTrans : clblasNoTrans;
  clblasTranspose transB = TB ? clblasTrans : clblasNoTrans;
  clblasSgemm(clblasRowMajor, transA, transB, M, N, K, ALPHA, *bufA, 0, lda,
              *bufB, 0, ldb, BETA, *bufC, offset, ldc, 1,
              reinterpret_cast<cl_command_queue *>(Kernel::GetQueue()), 0,
              nullptr, nullptr);
}

#else
void Blas::BlasSGemm(int TA, int TB, int M, int N, int K, float ALPHA,
                     const float *A, int lda, const float *B, int ldb,
                     float BETA, float *C, int offset, int ldc) {
  float *C_off = C + offset;
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; ++j) {
      C_off[i * ldc + j] *= BETA;
    }
  }
  if (!TA && !TB)
    SGemmNN(M, N, K, ALPHA, A, lda, B, ldb, C_off, ldc);
  else if (TA && !TB)
    SGemmTN(M, N, K, ALPHA, A, lda, B, ldb, C_off, ldc);
  else if (!TA && TB)
    SGemmNT(M, N, K, ALPHA, A, lda, B, ldb, C_off, ldc);
  else
    SGemmTT(M, N, K, ALPHA, A, lda, B, ldb, C_off, ldc);
}
#endif