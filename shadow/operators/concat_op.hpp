#ifndef SHADOW_OPERATORS_CONCAT_OP_HPP
#define SHADOW_OPERATORS_CONCAT_OP_HPP

#include "core/operator.hpp"

namespace Shadow {

class ConcatOp : public Operator {
 public:
  ConcatOp(const shadow::OpParam &op_param, Workspace *ws)
      : Operator(op_param, ws) {
    axis_ = get_single_argument<int>("axis", 1);
  }

  void Forward() override;

 private:
  int axis_;
};

namespace Vision {

template <typename T>
void Concat(const T *in_data, int count, int num_concats, int concat_size,
            int top_concat_axis, int bottom_concat_axis, int offset_concat_axis,
            T *out_data, Context *context);

}  // namespace Vision

}  // namespace Shadow

#endif  // SHADOW_OPERATORS_CONCAT_OP_HPP
