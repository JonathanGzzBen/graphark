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
#include "error.h"
#include "function_evaluator.h"
#include "program.h"

using namespace graphark::err;

auto glfw_error_callback(int error, const char *description) {
  std::cerr << "Error " << error << ": " << description << std::endl;
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
      graphark::Program::Create("shaders/vertex.glsl", "shaders/fragment.glsl")
          .or_else(print_err_and_abort_execution<graphark::Program>)
          .value();

  const graphark::Drawable axis = graphark::elements::get_axis_drawable();
  const graphark::Drawable grid = graphark::elements::get_grid_drawable();

  program.Use();

  const float aspect_ratio = get_aspect_ratio(window)
                                 .or_else(print_err_and_abort_execution<float>)
                                 .value();

  const auto m_projection =
      glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 10.0f);
  program.SetUniformMatrix("mProjection", m_projection)
      .or_else(print_err_and_abort_execution<void>);

  const auto m_view =
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.5f));
  program.SetUniformMatrix("mView", m_view)
      .or_else(print_err_and_abort_execution<void>);

  // const auto m_model =
  //     glm::rotate(glm::mat4(1.0f), 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
  // set_uniform_matrix(program, "mModel", m_model)
  //     .or_else(print_err_and_abort_execution<void>);

  /* Loop until the user closes the window */
  double time_elapsed = 0.0;
  auto func_index = 0;
  const graphark::Drawable lineas[] = {
      graphark::elements::get_function_line_drawable_from_str("x * x", -10, 10,
                                                              10),
      graphark::elements::get_function_line_drawable_from_str("x", -10, 10,
                                                              10)};
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

    program.SetUniformVector("vColor", glm::vec4(1.0, 0.5, 0.5, 1.0))
        .or_else(print_err_and_abort_execution<void>);
    glBindVertexArray(linea.vao);
    glDrawArrays(linea.draw_mode, 0, linea.vertex_count);

    program.SetUniformVector("vColor", glm::vec4(0.5, 0.5, 0.5, 1.0))
        .or_else(print_err_and_abort_execution<void>);
    glBindVertexArray(grid.vao);
    glDrawArrays(grid.draw_mode, 0, grid.vertex_count);

    program.SetUniformVector("vColor", glm::vec4(1.0, 1.0, 1.0, 1.0))
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