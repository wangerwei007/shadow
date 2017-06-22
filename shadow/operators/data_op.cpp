#include "data_op.hpp"
#include "core/image.hpp"

namespace Shadow {

void DataOp::Setup(VecBlobF *blobs) {
  auto *bottom = get_blob_by_name(*blobs, "in_blob");
  if (bottom != nullptr) {
    if (bottom->num() && bottom->num_axes() == 4) {
      bottoms_.push_back(bottom);
    } else {
      LOG(FATAL) << op_name_ << ": bottom blob("
                 << "in_blob"
                 << Util::format_vector(bottom->shape(), ",", "(", ")")
                 << ") dimension mismatch!";
    }
  } else {
    LOG(FATAL) << op_name_ << ": bottom blob("
               << "in_blob"
               << ") not exist!";
  }

  for (const auto &top_name : op_param_.top()) {
    auto *top = new BlobF(top_name);
    tops_.push_back(top);
    blobs->push_back(top);
  }

  const auto &data_param = op_param_.data_param();

  scale_ = data_param.scale();
  num_mean_ = 1;
  VecFloat mean_value;
  if (data_param.mean_value_size() > 1) {
    CHECK_EQ(data_param.mean_value_size(), bottoms_[0]->shape(1));
    for (const auto &val : data_param.mean_value()) {
      mean_value.push_back(val);
    }
    num_mean_ = mean_value.size();
  } else if (data_param.mean_value_size() == 1) {
    mean_value.push_back(data_param.mean_value(0));
  } else {
    mean_value.push_back(0);
  }
  mean_value_.reshape(num_mean_);
  mean_value_.set_data(mean_value.data(), num_mean_);
}

void DataOp::Reshape() {
  tops_[0]->reshape(bottoms_[0]->shape());

  DLOG(INFO) << op_name_ << ": "
             << Util::format_vector(bottoms_[0]->shape(), ",", "(", ")");
}

void DataOp::Forward() {
  Image::DataTransform(bottoms_[0]->data(), bottoms_[0]->shape(), scale_,
                       num_mean_, mean_value_.data(), tops_[0]->mutable_data());
}

void DataOp::Release() {
  mean_value_.clear();

  // DLOG(INFO) << "Free DataOp!";
}

}  // namespace Shadow
