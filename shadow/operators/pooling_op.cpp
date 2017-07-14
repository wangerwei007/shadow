#include "pooling_op.hpp"
#include "core/image.hpp"

namespace Shadow {

inline int pooling_out_size(int dim, int kernel_size, int stride, int pad) {
  return static_cast<int>(std::ceil(
             static_cast<float>(dim + 2 * pad - kernel_size) / stride)) +
         1;
}

void PoolingOp::Setup() {
  pool_type_ = arg_helper_.GetSingleArgument<int>("pool", 0);
  global_pooling_ =
      arg_helper_.GetSingleArgument<bool>("global_pooling", false);
  if (!global_pooling_) {
    CHECK(arg_helper_.HasArgument("kernel_size"));
    kernel_size_ = arg_helper_.GetSingleArgument<int>("kernel_size", 2);
    stride_ = arg_helper_.GetSingleArgument<int>("stride", 1);
    pad_ = arg_helper_.GetSingleArgument<int>("pad", 0);
  } else {
    kernel_size_ = bottoms_[0]->shape(2);
    stride_ = 1;
    pad_ = 0;
  }

#if defined(USE_CUDNN)
  cudnn::createPoolingDesc<float>(&pooling_desc_, pool_type_, &mode_,
                                  kernel_size_, kernel_size_, pad_, pad_,
                                  stride_, stride_);
  cudnn::createTensor4dDesc<float>(&bottom_desc_);
  cudnn::createTensor4dDesc<float>(&top_desc_);
#endif
}

void PoolingOp::Reshape() {
  int batch = bottoms_[0]->shape(0), in_c = bottoms_[0]->shape(1);
  int in_h = bottoms_[0]->shape(2), in_w = bottoms_[0]->shape(3);
  int out_h = pooling_out_size(in_h, kernel_size_, stride_, pad_);
  int out_w = pooling_out_size(in_w, kernel_size_, stride_, pad_);
  if (pad_) {
    if ((out_h - 1) * stride_ >= in_h + pad_) out_h--;
    if ((out_w - 1) * stride_ >= in_w + pad_) out_w--;
  }

  VecInt top_shape = bottoms_[0]->shape();
  top_shape[2] = out_h;
  top_shape[3] = out_w;
  tops_[0]->reshape(top_shape);

#if defined(USE_CUDNN)
  cudnn::setTensor4dDesc<float>(&bottom_desc_, batch, in_c, in_h, in_w);
  cudnn::setTensor4dDesc<float>(&top_desc_, batch, in_c, out_h, out_w);
#endif

  DLOG(INFO) << op_name_ << ": "
             << Util::format_vector(bottoms_[0]->shape(), ",", "(", ")")
             << " -> " << kernel_size_ << "x" << kernel_size_ << "_s" << stride_
             << " -> " << Util::format_vector(tops_[0]->shape(), ",", "(", ")");
}

void PoolingOp::Forward() {
#if defined(USE_CUDNN)
  CUDNN_CHECK(cudnnPoolingForward(
      Kernel::cudnn_handle_, pooling_desc_, cudnn::dataType<float>::one,
      bottom_desc_, bottoms_[0]->data(), cudnn::dataType<float>::zero,
      top_desc_, tops_[0]->mutable_data()));
  return;

#else
  Image::Pooling(bottoms_[0]->data(), bottoms_[0]->shape(), kernel_size_,
                 stride_, pad_, pool_type_, tops_[0]->shape(),
                 tops_[0]->mutable_data());
#endif
}

void PoolingOp::Release() {
#if defined(USE_CUDNN)
  if (pooling_desc_ != nullptr) {
    cudnnDestroyPoolingDescriptor(pooling_desc_);
    pooling_desc_ = nullptr;
  }
  if (bottom_desc_ != nullptr) {
    cudnnDestroyTensorDescriptor(bottom_desc_);
    bottom_desc_ = nullptr;
  }
  if (top_desc_ != nullptr) {
    cudnnDestroyTensorDescriptor(top_desc_);
    top_desc_ = nullptr;
  }
#endif

  // DLOG(INFO) << "Free PoolingOp!";
}

REGISTER_OPERATOR(Pooling, PoolingOp);

}  // namespace Shadow