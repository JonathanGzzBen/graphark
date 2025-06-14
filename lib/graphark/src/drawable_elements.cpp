#include "drawable_elements.h"

namespace graphark::elements {

template <typename T>
static auto map_linear(T value, T in_min, T in_max, T out_min, T out_max) -> T {
  return ((value - in_min) / (in_max - in_min)) * (out_max - out_min) + out_min;
}

static auto map_to_opengl_coordinates(float value, float x_min,
                                      float x_max) -> float {
  return map_linear<float>(value, x_min, x_max, -1.0f, 1.0f);
}

auto get_axis_drawable() -> graphark::Drawable {
  std::vector<float> line{};
  line.push_back(-1.0f);
  line.push_back(0.0f);
  line.push_back(1.0f);
  line.push_back(0.0f);
  line.push_back(0.0f);
  line.push_back(-1.0f);
  line.push_back(0.0f);
  line.push_back(1.0f);

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

auto get_grid_drawable() -> graphark::Drawable {
  std::vector<float> vertices{};
  auto func = [](float x) { return x; };

  // Horizontal lines
  for (int y_i = -10; y_i <= 10; y_i++) {
    vertices.push_back(-1.0f);
    vertices.push_back(static_cast<float>(y_i) * 0.1f);
    vertices.push_back(1.0f);
    vertices.push_back(static_cast<float>(y_i) * 0.1f);
  }
  // Vertical lines
  for (int x_i = -10; x_i <= 10; x_i++) {
    vertices.push_back(static_cast<float>(x_i) * 0.1f);
    vertices.push_back(-1.0f);
    vertices.push_back(static_cast<float>(x_i) * 0.1f);
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
  // glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  return graphark::Drawable{.vao = vao,
                            .vbo = vbo,
                            .draw_mode = GL_LINES,
                            .vertex_count =
                                static_cast<int>(vertices.size()) / 2};
}

auto get_function_line_drawable(const std::function<float(float)> &func,
                                float x_min,
                                float x_max) -> graphark::Drawable {

  std::vector<float> line{};

  float x = x_min;
  while (x <= x_max) {
    float y = func(x);
    line.push_back(map_to_opengl_coordinates(x, -10.0f, 10.0f));
    line.push_back(map_to_opengl_coordinates(y, -10.0f, 10.0f));
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
  // glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  return graphark::Drawable{.vao = vao,
                            .vbo = vbo,
                            .draw_mode = GL_LINE_STRIP,
                            .vertex_count = static_cast<int>(line.size()) / 2};
}

auto get_function_line_drawable_from_str(
    const std::string &expression_str, const int x_min, const int x_max,
    const int n_subdivisions) -> graphark::Drawable {

  std::vector<float> line{};

  graphark::FunctionEvaluator<float> evaluator(expression_str);
  for (int x_i = x_min * n_subdivisions; x_i <= x_max * n_subdivisions; x_i++) {
    float x = x_i * (1.0f / static_cast<float>(n_subdivisions));
    float y = evaluator.evaluate(x);
    line.push_back(map_to_opengl_coordinates(x, static_cast<float>(x_min),
                                             static_cast<float>(x_max)));
    line.push_back(map_to_opengl_coordinates(y, static_cast<float>(x_min),
                                             static_cast<float>(x_max)));
  }

  unsigned int vao;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, line.size() * sizeof(float), line.data(),
               GL_STATIC_DRAW);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  return graphark::Drawable{.vao = vao,
                            .vbo = vbo,
                            .draw_mode = GL_LINE_STRIP,
                            .vertex_count = static_cast<int>(line.size()) / 2};
}

} // namespace graphark::elements
