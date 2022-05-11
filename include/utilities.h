#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr
#include <cstring>    // For std::unique_ptr
#include <fstream>    // For std::unique_ptr

std::string string_format(const std::string fmt_str, ...);
std::string get_time_now();

bool doFileExists(std::string& name);