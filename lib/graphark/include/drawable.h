#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <GL/GL.h>

namespace graphark {

using Drawable = struct Drawable {
  unsigned int vao;
  unsigned int vbo;
  GLenum draw_mode;
  GLsizei vertex_count;
};

} // namespace graphark

#endif // DRAWABLE_H