#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tl/expected.hpp>

#include <cstdio>

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

template <typename T>
auto print_err_and_abort_execution(const Error &err) -> tl::expected<T, Error> {
  std::cerr << err.message << std::endl;
  std::exit(EXIT_FAILURE);
  return {}; // Unreachable, but required for compilation
}

auto compile_and_get_shader(GLenum shader_type, const std::string &filename)
    -> tl::expected<GLuint, Error> {
  // const auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
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
    // std::cerr << message << std::endl;
    // return 1;
    // Write the error to a log
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

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar *message, const void *userParam) {
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    return;
  std::cerr << "OpenGL Debug: " << message << std::endl;
}

int main(void) {
  GLFWwindow *window;

  /* Initialize the library */
  if (!glfwInit())
    return -1;

  /*   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); */
  glfwSetErrorCallback(glfw_error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
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
  glUseProgram(program);

  float vertices[] = {
      -0.5f, -0.5f, 0.0f, // One
      0.5f,  -0.5f, 0.0f, // Two
      0.0f,  0.5f,  0.0f  // Three
  };

  unsigned int vao;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  glUseProgram(program);
  glBindVertexArray(vao);
  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}