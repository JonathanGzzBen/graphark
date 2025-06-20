#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <GL/GL.h>
#include <vector>

namespace graphark {

class Drawable2D {
private:
  unsigned int m_vao;
  unsigned int m_vbo;
  GLenum m_draw_mode;
  GLsizei m_vertex_count;

public:
  Drawable2D(const unsigned int vao, const unsigned int vbo,
             const GLenum draw_mode, const GLsizei vertex_count)
      : m_vao{vao}, m_vbo{vbo}, m_draw_mode{draw_mode},
        m_vertex_count{vertex_count} {}

  Drawable2D(const std::vector<float> &vertices, const GLenum draw_mode)
      : m_draw_mode{draw_mode} {
    m_vertex_count = static_cast<GLsizei>(vertices.size() / 2);
    unsigned int vao;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);
    m_vao = vao;

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    m_vbo = vbo;

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
  }

  auto Draw() const -> void {
    glBindVertexArray(m_vao);
    glDrawArrays(m_draw_mode, 0, m_vertex_count);
    glBindVertexArray(0);
  }
};

} // namespace graphark

#endif // DRAWABLE_H