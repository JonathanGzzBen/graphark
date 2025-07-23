#include <CLI/CLI.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <ranges>
#include <string>
#include <tl/expected.hpp>

#include "graphark/camera.h"
#include "graphark/drawable2d.h"
#include "graphark/drawable_elements.h"
#include "graphark/error.h"
#include "graphark/program.h"

using namespace graphark::err;

auto glfw_error_callback(const int error, const char* description) {
  std::cerr << "Error " << error << ": " << description << std::endl;
}

void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id,
                              const GLenum severity, GLsizei length,
                              const GLchar* message, const void* userParam) {
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    return;
  std::cerr << "OpenGL Debug: " << message << std::endl;
}

auto get_aspect_ratio(GLFWwindow* window) -> tl::expected<float, Error> {
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

auto main(const int argc, char* argv[]) -> int {
  CLI::App app;
  std::string input_functions;
  app.add_option("functions", input_functions,
                 "Functions to graph separated by comma")
     ->required();

  CLI11_PARSE(app, argc, argv);

  const auto functions = ([&input_functions]() {
    auto functions_view =
        std::string_view(input_functions) |
        std::views::filter([](const auto s) { return s != ' '; }) |
        std::views::split(',');
    std::vector<std::string> result;
    for (const auto& function : functions_view) {
      result.emplace_back(function.begin(), function.end());
    }
    return result;
  })();

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
  GLFWwindow* window = glfwCreateWindow(500, 500, "Graphark", nullptr, nullptr);
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

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
  Camera cam(-10.0f, 10.0f, -10.0f, 10.0f);
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
  double delta_time = 0.0;
  while (!glfwWindowShouldClose(window)) {
    delta_time = get_delta();
    handle_input(static_cast<float>(delta_time));

    const auto m_projection =
        glm::ortho(cam.minX(), cam.maxX(), cam.minY(), cam.maxY(), -1.0f, 1.0f);

    const graphark::Drawable2D grid =
        graphark::elements::get_grid_drawable(cam);
    const graphark::Drawable2D axis =
        graphark::elements::get_axis_drawable(cam);

    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);

    program.SetUniformMatrix("mProjection", m_projection)
           .or_else(print_err_and_abort_execution<void>);

    program.SetUniformVector("vColor", glm::vec4(0.5, 0.5, 0.5, 1.0))
           .or_else(print_err_and_abort_execution<void>);
    grid.Draw();

    program.SetUniformVector("vColor", glm::vec4(1.0, 1.0, 1.0, 1.0))
           .or_else(print_err_and_abort_execution<void>);
    axis.Draw();

    program.SetUniformVector("vColor", glm::vec4(1.0, 0.5, 0.5, 1.0))
           .or_else(print_err_and_abort_execution<void>);
    for (const auto& function : functions) {
      const graphark::Drawable2D function_line =
          graphark::elements::get_function_line_drawable_from_str(function, cam,
            10);
      function_line.Draw();
    }

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}