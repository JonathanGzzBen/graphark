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

#include "drawable.h"
#include "drawable_elements.h"
#include "function_evaluator.h"

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
  GLFWwindow *window = glfwCreateWindow(640, 480, "Graphark", NULL, NULL);
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

  const graphark::Drawable axis = graphark::elements::get_axis_drawable();
  const graphark::Drawable grid = graphark::elements::get_grid_drawable();

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
  const graphark::Drawable lineas[] = {
      // get_function_line_drawable([](float x) { return x; })
      // get_function_line_drawable([](float x) { return x / 2; }),
      graphark::elements::get_function_line_drawable_from_str("x * x", -10, 10,
                                                              10),
      graphark::elements::get_function_line_drawable_from_str("x", -10, 10, 10)
      // graphark::elements::get_function_line_drawable_from_str("x", -10.0f,
      //                                                         10.0f, 100)
      // get_function_line_drawable_from_str("2x + 3")
      // graphark::elements::get_function_line_drawable_from_str(
      //     "(x^(2x) * (x^x)) + 1", -10.0f, 10.0f),
      // graphark::elements::get_function_line_drawable_from_str(
      //     "(x^(2x) * (x^x)) + 2", -10.0f, 10.0f),
      // graphark::elements::get_function_line_drawable_from_str(
      //     "(x^(2x) * (x^x)) + 3", -10.0f, 10.0f),
      // graphark::elements::get_function_line_drawable_from_str(
      //     "(x^(2x) * (x^x)) + 4", -10.0f, 10.0f),
      // graphark::elements::get_function_line_drawable_from_str(
      //     "(x^(2x) * (x^x)) + 5", -10.0f, 10.0f)
      // get_function_line_drawable_from_str("abs(sin(x))+5 e^(-x^(100))
      // cos(x)")
      // get_function_line_drawable([](float x) { return x * x; })
  };
  while (!glfwWindowShouldClose(window)) {
    time_elapsed += get_delta();

    if (time_elapsed >= 1.0) {
      func_index++;
      if (func_index >= (sizeof(lineas) / sizeof(lineas[0]))) {
        func_index = 0;
      }
      time_elapsed = 0.0;
    }
    const graphark::Drawable linea = lineas[func_index];
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