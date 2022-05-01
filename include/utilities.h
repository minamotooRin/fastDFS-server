#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr
#include <cstring>    // For std::unique_ptr

std::string string_format(const std::string fmt_str, ...);