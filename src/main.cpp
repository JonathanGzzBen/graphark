#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <fstream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <tl/expected.hpp>


using Error = struct Error {
  std::string message;
};

auto load_file(const std::string &filename)
    -> tl::expected<std::string, Error> {
  std::ifstream input_file(filename);
  std::stringstream contents_stream;
  if (!input_file.is_open() || input_file.fail()) {
    return tl::unexpected(
        Error{.message = "Could not open file \"" + filename + "\""});
  }
  contents_stream << input_file.rdbuf();
  return std::move(contents_stream.str());
}

template <typename T = void>
// auto print_err_and_abort_execution(Error &&err) -> tl::expected<T, Error> {
auto print_err_and_abort_execution(const Error &err) -> tl::expected<T, Error> {
  std::cerr << err.message << std::endl;
  std::exit(EXIT_FAILURE);
  return tl::unexpected(err);
}

auto compile_and_get_shader(GLenum shader_type, const std::string &filename)
    -> tl::expected<GLuint, Error> {
  const auto shader = glCreateShader(shader_type);
  const auto shader_source =
      load_file(filename)
          .or_else(print_err_and_abort_execution<std::string>)
          .value();
  const char *source = shader_source.c_str();
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  auto shader_compile_status = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compile_status);
  if (shader_compile_status != GL_TRUE) {
    char message[1024] = {0};
    GLsizei message_len = 0;
    glGetShaderInfoLog(shader, sizeof(message), &message_len, message);
    return tl::unexpected(Error{.message = message});
  }
  return shader;
}

auto get_linked_program(const std::string &vertex_shader_filename,
                        const std::string &fragment_shader_filename)
    -> tl::expected<GLuint, Error> {
  const auto vertex_shader =
      compile_and_get_shader(GL_VERTEX_SHADER, vertex_shader_filename)
          .or_else(print_err_and_abort_execution<GLenum>)
          .value();
  const auto fragment_shader =
      compile_and_get_shader(GL_FRAGMENT_SHADER, fragment_shader_filename)
          .or_else(print_err_and_abort_execution<GLenum>)
          .value();

  const auto program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  GLint link_status = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE) {
    GLsizei log_length = 0;
    GLchar message[1024];
    glGetProgramInfoLog(program, 1024, &log_length, message);
    return tl::unexpected(Error{.message = message});
  }
  return program;
}

template <typename... Args> static void printError(Args... args) noexcept {
  try {
    (std::cerr << ... << args);
  } catch (const std::exception &e) {
    std::fprintf(stderr, "printError failed: %s", e.what());
  }
}

auto glfw_error_callback(int error, const char *description) {
  printError("Error ", error, ": ", description, "\n");
}

auto set_uniform_matrix(const GLuint program, const std::string &uniform_name,
                        const glm::mat4 &matrix) -> tl::expected<void, Error> {
  const auto uniform_location =
      glGetUniformLocation(program, uniform_name.c_str());
  if (uniform_location < 0) {
    return tl::unexpected(
        Error{.message = "Could not get uniform location of \"" + uniform_name +
                         "\""});
  }
  glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(matrix));
  return {};
}

auto set_uniform_vector(const GLuint program, const std::string &uniform_name,
                        const glm::vec4 &vector) -> tl::expected<void, Error> {
  const auto uniform_location =
      glGetUniformLocation(program, uniform_name.c_str());
  if (uniform_location < 0) {
    return tl::unexpected(
        Error{.message = "Could not get uniform location of \"" + uniform_name +
                         "\""});
  }
  glUniform4fv(uniform_location, 1, glm::value_ptr(vector));
  return {};
}

void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar *message, const void *userParam) {
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    return;
  std::cerr << "OpenGL Debug: " << message << std::endl;
}

auto get_aspect_ratio(GLFWwindow *window) -> tl::expected<float, Error> {
  int window_width = -1;
  int window_height = -1;
  glfwGetWindowSize(window, &window_width, &window_height);
  if (window_width < 0 || window_height < 0)
    return tl::unexpected(Error{.message = "Could not get window size"});
  const float aspect_ratio =
      static_cast<float>(window_width) / static_cast<float>(window_height);
  return aspect_ratio;
}

template <typename T>
auto map_linear(T value, T in_min, T in_max, T out_min, T out_max) -> T {
  return ((value - in_min) / (in_max - in_min)) * (out_max - out_min) + out_min;
}

static constexpr int X_MIN = -10;
static constexpr int X_MAX = 10;
auto map_to_opengl_coordinates(float value) -> float {
  // static constexpr int Y_MIN = -100;
  // static constexpr int Y_MAX = 100;
  return map_linear<float>(value, X_MIN, X_MAX, -1.0f, 1.0f);
}

using Drawable = struct Drawable {
  unsigned int vao;
  unsigned int vbo;
  GLenum draw_mode;
  GLsizei vertex_count;
};

auto get_axis_drawable() -> Drawable {
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

  return Drawable{.vao = vao,
                  .vbo = vbo,
                  .draw_mode = GL_LINES,
                  .vertex_count = static_cast<int>(line.size()) / 2};
}

auto get_grid_drawable() -> Drawable {
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

  return Drawable{.vao = vao,
                  .vbo = vbo,
                  .draw_mode = GL_LINES,
                  .vertex_count = static_cast<int>(vertices.size()) / 2};
}

auto get_function_line_drawable(const std::function<float(float)> &func)
    -> Drawable {

  std::vector<float> line{};

  float x = X_MIN;
  while (x <= X_MAX) {
    float y = func(x);
    line.push_back(map_to_opengl_coordinates(x));
    line.push_back(map_to_opengl_coordinates(y));
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

  return Drawable{.vao = vao,
                  .vbo = vbo,
                  .draw_mode = GL_LINE_STRIP,
                  .vertex_count = static_cast<int>(line.size()) / 2};
}

auto get_delta() -> double {
  double currentTime = glfwGetTime();
  static double lastTime = currentTime;
  double deltaTime = currentTime - lastTime;
  lastTime = currentTime;
  return deltaTime;
}

auto main(void) -> int {

  /* Initialize the library */
  if (!glfwInit())
    return -1;

  glfwSetErrorCallback(glfw_error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

  /* Create a windowed mode window and its OpenGL context */
  GLFWwindow *window = glfwCreateWindow(640, 480, WINDOW_TITLE, NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    /* Problem: glewInit failed, something is seriously wrong. */
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }
  fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(glDebugCallback, nullptr);

  const auto program =
      get_linked_program("shaders/vertex.glsl", "shaders/fragment.glsl")
          .or_else(print_err_and_abort_execution<GLuint>)
          .value();

  const Drawable axis = get_axis_drawable();
  const Drawable grid = get_grid_drawable();

  glUseProgram(program);

  const float aspect_ratio = get_aspect_ratio(window)
                                 .or_else(print_err_and_abort_execution<float>)
                                 .value();

  const auto m_projection =
      glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 10.0f);
  set_uniform_matrix(program, "mProjection", m_projection)
      .or_else(print_err_and_abort_execution<void>);

  const auto m_view =
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.5f));
  set_uniform_matrix(program, "mView", m_view)
      .or_else(print_err_and_abort_execution<void>);

  // const auto m_model =
  //     glm::rotate(glm::mat4(1.0f), 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
  // set_uniform_matrix(program, "mModel", m_model)
  //     .or_else(print_err_and_abort_execution<void>);

  /* Loop until the user closes the window */
  double time_elapsed = 0.0;
  auto func_index = 0;
  const Drawable lineas[] = {
      get_function_line_drawable([](float x) { return x; }),
      get_function_line_drawable([](float x) { return x / 2; }),
      get_function_line_drawable([](float x) { return x * x; })};
  while (!glfwWindowShouldClose(window)) {
    time_elapsed += get_delta();
    std::cout << "Delta time: " << get_delta() << std::endl;

    if (time_elapsed >= 1.0) {
      func_index++;
      if (func_index >= (sizeof(lineas) / sizeof(lineas[0]))) {
        func_index = 0;
      }
      time_elapsed = 0.0;
    }
    const Drawable linea = lineas[func_index];
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);

    set_uniform_vector(program, "vColor", glm::vec4(1.0, 0.5, 0.5, 1.0))
        .or_else(print_err_and_abort_execution<void>);
    glBindVertexArray(linea.vao);
    glDrawArrays(linea.draw_mode, 0, linea.vertex_count);

    set_uniform_vector(program, "vColor", glm::vec4(0.5, 0.5, 0.5, 1.0))
        .or_else(print_err_and_abort_execution<void>);
    glBindVertexArray(grid.vao);
    glDrawArrays(grid.draw_mode, 0, grid.vertex_count);

    set_uniform_vector(program, "vColor", glm::vec4(1.0, 1.0, 1.0, 1.0))
        .or_else(print_err_and_abort_execution<void>);
    glBindVertexArray(axis.vao);
    glDrawArrays(axis.draw_mode, 0, axis.vertex_count);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}