#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>

class Camera {

private:
  float x_min_;
  float x_max_;
  float y_min_;
  float y_max_;

public:
  Camera(float x_min, float x_max, float y_min, float y_max)
      : x_min_{x_min}, x_max_{x_max}, y_min_{y_min}, y_max_{y_max} {}

  auto minX() const { return x_min_; }
  auto maxX() const { return x_max_; }
  auto minY() const { return y_min_; }
  auto maxY() const { return y_max_; }

  auto pan(float x, float y) {
    x_min_ += x;
    x_max_ += x;
    y_min_ += y;
    y_max_ += y;
  }

  auto zoom(float factor) {
    const double width = (x_max_ - x_min_);
    const double height = (y_max_ - y_min_);
    const double new_width = width * factor;
    const double new_height = height * factor;

    const double center_x = (x_min_ + x_max_) / 2.0;
    const double center_y = (y_min_ + y_max_) / 2.0;

    x_min_ = static_cast<float>(center_x - (new_width / 2.0));
    x_max_ = static_cast<float>(center_x + (new_width / 2.0));
    y_min_ = static_cast<float>(center_y - (new_height / 2.0));
    y_max_ = static_cast<float>(center_y + (new_height / 2.0));
  }
};

#endif // CAMERA_H