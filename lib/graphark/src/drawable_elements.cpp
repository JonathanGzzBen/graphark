#include "drawable_elements.h"

namespace graphark::elements {

auto get_axis_drawable(const Camera &cam) -> graphark::Drawable {
  std::vector<float> line{};

  // Vertical line
  if (-1.0f <= cam.normY(0.0f) && cam.normY(0.0f) <= 1.0f) {
    line.push_back(-1.0f);
    line.push_back(cam.normY(0.0f));
    line.push_back(1.0f);
    line.push_back(cam.normY(0.0f));
  }

  // Horizontal line
  if (-1.0f <= cam.normX(0.0f) && cam.normX(0.0f) <= 1.0f) {
    line.push_back(cam.normX(0.0f));
    line.push_back(-1.0f);
    line.push_back(cam.normX(0.0f));
    line.push_back(1.0f);
  }

  unsigned int vao;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, line.size() * sizeof(float), line.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  return graphark::Drawable{.vao = vao,
                            .vbo = vbo,
                            .draw_mode = GL_LINES,
                            .vertex_count = static_cast<int>(line.size()) / 2};
}

auto get_grid_drawable(const Camera &cam) -> graphark::Drawable {
  std::vector<float> vertices{};
  float step = 1.0f;
  // Horizontal lines
  for (float y = std::floor(cam.minY()); y <= cam.maxY(); y += step) {
    if (y < cam.minY() || cam.maxY() < y)
      continue;
    float ny = cam.normY(y);
    vertices.push_back(-1.0f);
    vertices.push_back(ny);
    vertices.push_back(1.0f);
    vertices.push_back(ny);
  }

  // Vertical lines
  for (float x = std::floor(cam.minX()); x <= cam.maxX(); x += step) {
    if (x < cam.minX() || cam.maxX() < x)
      continue;
    float nx = cam.normX(x);
    vertices.push_back(nx);
    vertices.push_back(-1.0f);
    vertices.push_back(nx);
    vertices.push_back(1.0f);
  }

  unsigned int vao;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  return graphark::Drawable{.vao = vao,
                            .vbo = vbo,
                            .draw_mode = GL_LINES,
                            .vertex_count =
                                static_cast<int>(vertices.size()) / 2};
}

auto get_function_line_drawable(const std::function<float(float)> &func,
                                const Camera &cam) -> graphark::Drawable {
  std::vector<float> line{};

  float x = cam.minX();
  while (x <= cam.maxX()) {
    float y = func(x);

    line.push_back(cam.normX(x));
    line.push_back(cam.normY(y));
    x += 0.1f;
  }

  unsigned int vao;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, line.size() * sizeof(float), line.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  return graphark::Drawable{.vao = vao,
                            .vbo = vbo,
                            .draw_mode = GL_LINE_STRIP,
                            .vertex_count = static_cast<int>(line.size()) / 2};
}

auto get_function_line_drawable_from_str(
    const std::string &expression_str, const Camera &cam,
    const int n_subdivisions) -> graphark::Drawable {

  std::vector<float> line{};

  graphark::FunctionEvaluator<float> evaluator(expression_str);
  for (int x_i = cam.minX() * n_subdivisions;
       x_i <= cam.maxX() * n_subdivisions; x_i++) {
    float x = x_i * (1.0f / static_cast<float>(n_subdivisions));
    float y = evaluator.evaluate(x);
    line.push_back(cam.normX(x));
    line.push_back(cam.normY(y));
  }

  unsigned int vao;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, line.size() * sizeof(float), line.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  return graphark::Drawable{.vao = vao,
                            .vbo = vbo,
                            .draw_mode = GL_LINE_STRIP,
                            .vertex_count = static_cast<int>(line.size()) / 2};
}

} // namespace graphark::elements
