#include "cudnn.hpp"

namespace Shadow {

#if defined(USE_CUDNN)

namespace cudnn {

float dataType<float>::oneval = 1.0;
float dataType<float>::zeroval = 0.0;
const void* dataType<float>::one = static_cast<void*>(&dataType<float>::oneval);
const void* dataType<float>::zero =
    static_cast<void*>(&dataType<float>::zeroval);

double dataType<double>::oneval = 1.0;
double dataType<double>::zeroval = 0.0;
const void* dataType<double>::one =
    static_cast<void*>(&dataType<double>::oneval);
const void* dataType<double>::zero =
    static_cast<void*>(&dataType<double>::zeroval);

}  // namespace cudnn

namespace Kernel {

cudnnHandle_t cudnn_handle_ = nullptr;

}  // namespace Kernel

#endif

}  // namespace Shadow
