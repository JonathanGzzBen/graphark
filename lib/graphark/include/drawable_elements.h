#ifndef DRAWABLE_ELEMENTS_H
#define DRAWABLE_ELEMENTS_H

#include <GL/glew.h>

#include "drawable.h"
#include "function_evaluator.h"
#include <functional>
#include <vector>

namespace graphark::elements {

auto get_axis_drawable() -> graphark::Drawable;

auto get_grid_drawable() -> graphark::Drawable;

auto get_function_line_drawable(const std::function<float(float)> &func,
                                float x_min, float x_max) -> graphark::Drawable;

auto get_function_line_drawable_from_str(
    const std::string &expression_str, const int x_min, const int x_max,
    const int n_subdivisions) -> graphark::Drawable;

} // namespace graphark::elements

#endif // DRAWABLE_ELEMENTS_H