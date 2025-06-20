#include <CLI/CLI.hpp>
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

#include "camera.h"
#include "drawable2d.h"
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

auto main(int argc, char *argv[]) -> int {
  CLI::App app;
  std::string input_function;
  app.add_option("function", input_function, "Function to graph")->required();
  CLI11_PARSE(app, argc, argv);

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

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
  Camera cam;
  const auto handle_input = [&window, &cam](float delta_time) {
    const auto is_pressed = [&window](int key) {
      return glfwGetKey(window, key) == GLFW_PRESS;
    };

    float pan_speed = (cam.maxX() - cam.minX()) * 0.5f;
    float zoom_factor = 2.5f;

    if (is_pressed(GLFW_KEY_LEFT)) {
      cam.pan(-pan_speed * delta_time, 0.0f);
    }
    if (is_pressed(GLFW_KEY_RIGHT)) {
      cam.pan(pan_speed * delta_time, 0.0f);
    }
    if (is_pressed(GLFW_KEY_UP)) {
      cam.pan(0.0f, pan_speed * delta_time);
    }
    if (is_pressed(GLFW_KEY_DOWN)) {
      cam.pan(0.0f, -pan_speed * delta_time);
    }
    if (is_pressed(GLFW_KEY_EQUAL)) {
      cam.zoom(static_cast<float>(
          std::pow(1.0 / static_cast<double>(zoom_factor), delta_time)));
    }
    if (is_pressed(GLFW_KEY_MINUS)) {
      cam.zoom(std::pow(zoom_factor, delta_time));
    }
  };

  /* Loop until the user closes the window */
  auto func_index = 0;
  double delta_time = 0.0;
  while (!glfwWindowShouldClose(window)) {
    delta_time = get_delta();
    handle_input(static_cast<float>(delta_time));

    const graphark::Drawable2D axis =
        graphark::elements::get_axis_drawable(cam);
    const graphark::Drawable2D grid =
        graphark::elements::get_grid_drawable(cam);
    const graphark::Drawable2D function_line =
        graphark::elements::get_function_line_drawable_from_str(input_function,
                                                                cam, 100);
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);

    program.SetUniformVector("vColor", glm::vec4(0.5, 0.5, 0.5, 1.0))
        .or_else(print_err_and_abort_execution<void>);
    grid.Draw();

    program.SetUniformVector("vColor", glm::vec4(1.0, 1.0, 1.0, 1.0))
        .or_else(print_err_and_abort_execution<void>);
    axis.Draw();

    program.SetUniformVector("vColor", glm::vec4(1.0, 0.5, 0.5, 1.0))
        .or_else(print_err_and_abort_execution<void>);
    function_line.Draw();

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}