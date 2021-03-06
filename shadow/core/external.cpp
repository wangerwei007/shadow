#include "external.hpp"

namespace Shadow {

#if defined(USE_CUDNN)

namespace cudnn {

float dataType<float>::oneval = 1.0;
float dataType<float>::zeroval = 0.0;
const void *dataType<float>::one =
    static_cast<void *>(&dataType<float>::oneval);
const void *dataType<float>::zero =
    static_cast<void *>(&dataType<float>::zeroval);

double dataType<double>::oneval = 1.0;
double dataType<double>::zeroval = 0.0;
const void *dataType<double>::one =
    static_cast<void *>(&dataType<double>::oneval);
const void *dataType<double>::zero =
    static_cast<void *>(&dataType<double>::zeroval);

}  // namespace cudnn

#endif

}  // namespace Shadow

namespace Shadow {

#if defined(USE_DNNL)

namespace idnnl {

dnnl::convolution_forward::desc create_convolution_desc(
    const dnnl::memory::desc &src_desc, const dnnl::memory::desc &weight_desc,
    const dnnl::memory::desc &bias_desc, const dnnl::memory::desc &dst_desc,
    int pad_h, int pad_w, int stride_h, int stride_w, int dilation_h,
    int dilation_w) {
  if (dilation_h == 1 && dilation_w == 1) {
    return dnnl::convolution_forward::desc(
        dnnl::prop_kind::forward_inference, dnnl::algorithm::convolution_auto,
        src_desc, weight_desc, bias_desc, dst_desc, {stride_h, stride_w},
        {pad_h, pad_w}, {pad_h, pad_w});
  } else {
    return dnnl::convolution_forward::desc(
        dnnl::prop_kind::forward_inference, dnnl::algorithm::convolution_auto,
        src_desc, weight_desc, bias_desc, dst_desc, {stride_h, stride_w},
        {dilation_h - 1, dilation_w - 1}, {pad_h, pad_w}, {pad_h, pad_w});
  }
}

void convolution_forward(void *dnnl_engine, void *dnnl_stream,
                         const dnnl::convolution_forward::desc &conv_desc,
                         const void *src_data, const void *weight_data,
                         const void *bias_data, void *dst_data,
                         int activate_type) {
  const auto *engine = (dnnl::engine *)dnnl_engine;
  auto *stream = (dnnl::stream *)dnnl_stream;
  auto conv_primitive_desc =
      dnnl::convolution_forward::primitive_desc(conv_desc, *engine);
  if (activate_type == 1) {
    dnnl::post_ops ops;
    ops.append_eltwise(1.f, dnnl::algorithm::eltwise_relu, 0.f, 1.f);
    dnnl::primitive_attr attr;
    attr.set_post_ops(ops);
    conv_primitive_desc =
        dnnl::convolution_forward::primitive_desc(conv_desc, attr, *engine);
  }
  const auto &src_mem = dnnl::memory(conv_primitive_desc.src_desc(), *engine,
                                     const_cast<void *>(src_data));
  const auto &weight_mem =
      dnnl::memory(conv_primitive_desc.weights_desc(), *engine,
                   const_cast<void *>(weight_data));
  const auto &bias_mem = dnnl::memory(conv_primitive_desc.bias_desc(), *engine,
                                      const_cast<void *>(bias_data));
  const auto &dst_mem =
      dnnl::memory(conv_primitive_desc.dst_desc(), *engine, dst_data);
  dnnl::convolution_forward(conv_primitive_desc)
      .execute(*stream, {{DNNL_ARG_SRC, src_mem},
                         {DNNL_ARG_WEIGHTS, weight_mem},
                         {DNNL_ARG_BIAS, bias_mem},
                         {DNNL_ARG_DST, dst_mem}});
}

}  // namespace idnnl

#endif

}  // namespace Shadow
