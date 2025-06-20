#include "drawable_elements.h"

namespace graphark::elements {

auto get_axis_drawable(const Camera &cam) -> graphark::Drawable2D {
  std::vector<float> lines{};

  // Vertical line
  if (-1.0f <= cam.normY(0.0f) && cam.normY(0.0f) <= 1.0f) {
    lines.push_back(-1.0f);
    lines.push_back(cam.normY(0.0f));
    lines.push_back(1.0f);
    lines.push_back(cam.normY(0.0f));
  }

  // Horizontal line
  if (-1.0f <= cam.normX(0.0f) && cam.normX(0.0f) <= 1.0f) {
    lines.push_back(cam.normX(0.0f));
    lines.push_back(-1.0f);
    lines.push_back(cam.normX(0.0f));
    lines.push_back(1.0f);
  }

  return graphark::Drawable2D(lines, GL_LINES);
}

auto get_grid_drawable(const Camera &cam) -> graphark::Drawable2D {
  std::vector<float> vertices{};

  // Horizontal lines
  for (int y = static_cast<int>(std::floor(cam.minY()));
       y <= static_cast<int>(cam.maxY()); y++) {
    if (y < cam.minY())
      continue;
    float ny = cam.normY(static_cast<float>(y));
    vertices.push_back(-1.0f);
    vertices.push_back(ny);
    vertices.push_back(1.0f);
    vertices.push_back(ny);
  }
  // Vertical lines
  for (int x = static_cast<int>(std::floor(cam.minX()));
       x <= static_cast<int>(cam.maxX()); x++) {
    if (x < cam.minX())
      continue;
    float nx = cam.normX(static_cast<float>(x));
    vertices.push_back(nx);
    vertices.push_back(-1.0f);
    vertices.push_back(nx);
    vertices.push_back(1.0f);
  }

  return graphark::Drawable2D(vertices, GL_LINES);
}

auto get_function_line_drawable_from_str(
    const std::string &expression_str, const Camera &cam,
    const int n_subdivisions) -> graphark::Drawable2D {

  std::vector<float> line{};

  graphark::FunctionEvaluator<float> evaluator(expression_str);

  float step_size = 1.0f / n_subdivisions;
  size_t num_steps =
      static_cast<size_t>(std::ceil(cam.maxX() - cam.minX()) / step_size);

  for (size_t i = 0; i <= num_steps; i++) {
    float x = cam.minX() + (i * step_size);
    float y = evaluator.evaluate(x);
    line.push_back(cam.normX(x));
    line.push_back(cam.normY(y));
  }

  return graphark::Drawable2D(line, GL_LINE_STRIP);
}

} // namespace graphark::elements
