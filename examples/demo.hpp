#ifndef SHADOW_DEMO_HPP
#define SHADOW_DEMO_HPP

#include "ssd/ssd.hpp"

#include "shadow/util/boxes.hpp"
#include "shadow/util/jimage.hpp"

class Demo {
 public:
  void Setup(const std::string &model_file, int batch = 1) {
    method_.Setup(model_file, batch);
  }
  void Release() {
    method_.Release();
    im_ini_.Release();
    Bboxes_.clear();
  }

  void Test(const std::string &image_file);
  void BatchTest(const std::string &list_file, bool image_write = false);
#if defined(USE_OpenCV)
  void VideoTest(const std::string &video_file, bool video_show = true,
                 bool video_write = false);
  void CameraTest(int camera, bool video_write = false);
#endif

 private:
  void Predict(const JImage &image, const VecRectF &rois,
               std::vector<VecBoxF> *Bboxes) {
    method_.Predict(image, rois, Bboxes);
  }

#if defined(USE_OpenCV)
  void CaptureTest(cv::VideoCapture *capture, const std::string &window_name,
                   bool video_show, cv::VideoWriter *writer);
  void DrawDetections(const VecBoxF &boxes, cv::Mat *im_mat,
                      bool console_show = true);
#endif

  void PrintDetections(const std::string &im_name, const VecBoxF &boxes,
                       std::ofstream *file);

  METHOD method_;
  JImage im_ini_;
  std::vector<VecBoxF> Bboxes_;
  Timer timer_;
};

#endif  // SHADOW_DEMO_HPP
