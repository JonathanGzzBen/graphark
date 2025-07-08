#include "graphark/drawable_elements.h"

#include <vector>

namespace graphark::elements {
auto get_axis_drawable(const Camera& cam) -> graphark::Drawable2D {
  std::vector<float> lines{};

  // Vertical line
  lines.push_back(cam.minX());
  lines.push_back(0.0f);
  lines.push_back(cam.maxX());
  lines.push_back(0.0f);

  // Horizontal line
  lines.push_back(0.0f);
  lines.push_back(cam.minY());
  lines.push_back(0.0f);
  lines.push_back(cam.maxY());

  return {lines, GL_LINES};
}

auto get_grid_drawable(const Camera& cam) -> graphark::Drawable2D {
  std::vector<float> vertices{};

  // Vertical lines
  const int x_start = static_cast<int>(std::floor(cam.minX()));
  const int x_end = static_cast<int>(std::ceil(cam.maxX()));
  for (int x = x_start; x <= x_end; x++) {
    vertices.push_back(static_cast<float>(x));
    vertices.push_back(cam.minY());
    vertices.push_back(static_cast<float>(x));
    vertices.push_back(cam.maxY());
  }
  // Horizontal lines
  const int y_start = static_cast<int>(std::floor(cam.minY()));
  const int y_end = static_cast<int>(std::ceil(cam.maxY()));
  for (int y = y_start; y <= y_end; y++) {
    vertices.push_back(cam.minX());
    vertices.push_back(static_cast<float>(y));
    vertices.push_back(cam.maxX());
    vertices.push_back(static_cast<float>(y));
  }

  return {vertices, GL_LINES};
}

auto get_function_line_drawable_from_str(
    const std::string& expression_str, const Camera& cam,
    const int n_subdivisions) -> graphark::Drawable2D {
  std::vector<float> line{};

  graphark::FunctionEvaluator<float> evaluator(expression_str);

  const float step_size = 1.0f / static_cast<float>(n_subdivisions);
  float x = cam.minX();
  while (x <= cam.maxX()) {
    float y = evaluator.evaluate(x);
    line.push_back(x);
    line.push_back(y);
    x += step_size;
  }

  return {line, GL_LINE_STRIP};
}
} // namespace graphark::elements