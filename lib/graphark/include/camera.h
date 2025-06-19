#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>

class Camera {

private:
  float min_x = -10.0f;
  float max_x = 10.0f;
  float min_y = -10.0f;
  float max_y = 10.0f;

  template <typename T>
  static auto map_linear(T value, T in_min, T in_max, T out_min,
                         T out_max) -> T {
    return ((value - in_min) / (in_max - in_min)) * (out_max - out_min) +
           out_min;
  }

  static auto map_to_opengl_coordinates(float value, float x_min,
                                        float x_max) -> float {
    return map_linear<float>(value, x_min, x_max, -1.0f, 1.0f);
  }

public:
  auto normX(float x) const {
    return map_to_opengl_coordinates(x, min_x, max_x);
  }
  auto normY(float y) const {
    return map_to_opengl_coordinates(y, min_y, max_y);
  }
  auto minX() const { return min_x; }
  auto maxX() const { return max_x; }
  auto minY() const { return min_y; }
  auto maxY() const { return max_y; }
  auto width() const { return max_x - min_x; }
  auto height() const { return max_y - min_y; }

  auto pan(float x, float y) {
    min_x += x;
    max_x += x;
    min_y += y;
    max_y += y;
  }

  auto zoom(float factor) {
    double cx = (min_x + max_x) / 2.0f;
    double cy = (min_y + max_y) / 2.0f;
    double new_width = width() * factor;
    double new_height = height() * factor;

    min_x = cx - (new_width / 2.0f);
    max_x = cx + (new_width / 2.0f);
    min_y = cy - (new_height / 2.0f);
    max_y = cy + (new_height / 2.0f);

    // const auto new_width = std::abs(width * factor);
    // const auto new_height = std::abs(height * factor);

    // min_x = ((new_width) / 2) * -1;
    // max_x = ((new_width) / 2);
    // min_y = ((new_height) / 2) * -1;
    // max_y = ((new_height) / 2);
  }
};

#endif // CAMERA_H