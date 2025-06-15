#ifndef PROGRAM_H
#define PROGRAM_H

#include <GL/glew.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <tl/expected.hpp>

#include "error.h"

using namespace graphark::err;

namespace graphark {

static auto
load_file(const std::string &filename) -> tl::expected<std::string, Error> {
  std::ifstream input_file(filename);
  std::stringstream contents_stream;
  if (!input_file.is_open() || input_file.fail()) {
    return tl::unexpected(
        Error{.message = "Could not open file \"" + filename + "\""});
  }
  contents_stream << input_file.rdbuf();
  return std::move(contents_stream.str());
}

static auto compile_and_get_shader(GLenum shader_type,
                                   const std::string &filename)
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

class Program {
private:
  unsigned int m_program;

  Program(const unsigned int vertex_shader,
          const unsigned int fragment_shader) {
    {
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
        // return tl::unexpected(Error{.message = message});
      }
      m_program = program;
    }
  }

public:
  static auto Create(const std::string &vertex_shader_filename,
                     const std::string &fragment_shader_filename)
      -> tl::expected<Program, Error> {

    const auto vertex_shader =
        compile_and_get_shader(GL_VERTEX_SHADER, vertex_shader_filename)
            .or_else(print_err_and_abort_execution<GLenum>)
            .value();
    const auto fragment_shader =
        compile_and_get_shader(GL_FRAGMENT_SHADER, fragment_shader_filename)
            .or_else(print_err_and_abort_execution<GLenum>)
            .value();

    return Program(vertex_shader, fragment_shader);
  }

  auto
  SetUniformMatrix(const std::string &uniform_name,
                   const glm::mat4 &matrix) const -> tl::expected<void, Error> {
    const auto uniform_location =
        glGetUniformLocation(m_program, uniform_name.c_str());
    if (uniform_location < 0) {
      return tl::unexpected(
          Error{.message = "Could not get uniform location of \"" +
                           uniform_name + "\""});
    }
    glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(matrix));
    return {};
  }

  auto
  SetUniformVector(const std::string &uniform_name,
                   const glm::vec4 &vector) const -> tl::expected<void, Error> {
    const auto uniform_location =
        glGetUniformLocation(m_program, uniform_name.c_str());
    if (uniform_location < 0) {
      return tl::unexpected(
          Error{.message = "Could not get uniform location of \"" +
                           uniform_name + "\""});
    }
    glUniform4fv(uniform_location, 1, glm::value_ptr(vector));
    return {};
  }

  auto ProgramID() const -> unsigned int { return m_program; }
  auto Use() const -> void { glUseProgram(m_program); }
};

} // namespace graphark

#endif // PROGRAM_H