#ifndef DRAWABLE_ELEMENTS_H
#define DRAWABLE_ELEMENTS_H

#include <GL/glew.h>
#include <functional>
#include <vector>

#include "camera.h"
#include "drawable2d.h"
#include "function_evaluator.h"

namespace graphark::elements {

auto get_axis_drawable(const Camera &cam) -> graphark::Drawable2D;

auto get_grid_drawable(const Camera &cam) -> graphark::Drawable2D;

auto get_function_line_drawable_from_str(
    const std::string &expression_str, const Camera &cam,
    const int n_subdivisions) -> graphark::Drawable2D;

} // namespace graphark::elements

#endif // DRAWABLE_ELEMENTS_H