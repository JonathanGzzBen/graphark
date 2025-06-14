#ifndef FUNCTION_EVALUATOR_H
#define FUNCTION_EVALUATOR_H

#include "exprtk.hpp"
#include <string>

namespace graphark {

template <typename T> class FunctionEvaluator {
private:
  T m_x;
  exprtk::symbol_table<T> m_symbol_table;
  exprtk::expression<T> m_expression;
  exprtk::parser<T> m_parser;

public:
  explicit FunctionEvaluator(const std::string &expression_str) {
    m_symbol_table.add_variable("x", m_x);
    m_symbol_table.add_constants();

    m_expression.register_symbol_table(m_symbol_table);

    m_parser.compile(expression_str, m_expression);
  }

  auto evaluate(T x) -> T {
    m_x = x;
    return m_expression.value();
  }
};

} // namespace graphark

#endif // FUNCTION_EVALUATOR_H