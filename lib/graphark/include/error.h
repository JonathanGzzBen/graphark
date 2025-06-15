#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include <string>
#include <tl/expected.hpp>

namespace graphark::err {

using Error = struct Error {
  std::string message;
};

template <typename T = void>
// auto print_err_and_abort_execution(Error &&err) -> tl::expected<T, Error> {
auto print_err_and_abort_execution(const Error &err) -> tl::expected<T, Error> {
  std::cerr << err.message << std::endl;
  std::exit(EXIT_FAILURE);
  return tl::unexpected(err);
}

} // namespace graphark::err

#endif // ERROR_H