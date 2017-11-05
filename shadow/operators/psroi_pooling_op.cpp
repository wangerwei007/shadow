#include "psroi_pooling_op.hpp"
#include "core/vision.hpp"

namespace Shadow {

void PSROIPoolingOp::Reshape() {
  const auto *bottom_fea = bottoms<float>(0);
  const auto *bottom_roi = bottoms<float>(1);
  auto *top = mutable_tops<float>(0);

  CHECK_NE(bottom_fea, top);

  int num_rois = bottom_roi->shape(0);

  top->reshape({num_rois, output_dim_, pooled_h_, pooled_w_});

  VecString str;
  for (int i = 0; i < bottoms_size(); ++i) {
    const auto *bottom = bottoms<float>(i);
    str.push_back(
        Util::format_vector(bottom->shape(), ",", bottom->name() + "(", ")"));
  }
  DLOG(INFO) << op_name_ << "(" << op_type_
             << "): " << Util::format_vector(str, " + ") << " -> "
             << top->name() << Util::format_vector(top->shape(), ",", "(", ")");
}

void PSROIPoolingOp::Forward() {
  const auto *bottom_fea = bottoms<float>(0);
  const auto *bottom_roi = bottoms<float>(1);
  auto *top = mutable_tops<float>(0);

  int num_rois = bottom_roi->shape(0);

  Vision::PSROIPooling(bottom_fea->data(), bottom_fea->shape(),
                       bottom_roi->data(), num_rois, output_dim_, group_size_,
                       pooled_h_, pooled_w_, spatial_scale_,
                       top->mutable_data());
}

REGISTER_OPERATOR(PSROIPooling, PSROIPoolingOp);

}  // namespace Shadow
