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
  float step = 1.0f;
  // Horizontal lines
  for (float y = std::floor(cam.minY()); y <= cam.maxY(); y += step) {
    if (y < cam.minY())
      continue;
    float ny = cam.normY(y);
    vertices.push_back(-1.0f);
    vertices.push_back(ny);
    vertices.push_back(1.0f);
    vertices.push_back(ny);
  }

  // Vertical lines
  for (float x = std::floor(cam.minX()); x <= cam.maxX(); x += step) {
    if (x < cam.minX())
      continue;
    float nx = cam.normX(x);
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
  for (int x_i = cam.minX() * n_subdivisions;
       x_i <= cam.maxX() * n_subdivisions; x_i++) {
    float x = x_i * (1.0f / static_cast<float>(n_subdivisions));
    float y = evaluator.evaluate(x);
    line.push_back(cam.normX(x));
    line.push_back(cam.normY(y));
  }
  return graphark::Drawable2D(line, GL_LINE_STRIP);
}

} // namespace graphark::elements
