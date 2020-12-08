#pragma once

#include <string>

/**
 * See: https://pybind11.readthedocs.io/en/stable/classes.html#enumerations-and-internal-types
 * this set up will let Python see:
 *
 * LogLevel.Trace = the enum value
 * LogLevel.to_string(LogLevel.Trace) = the string value
 *
 * But it's not actually constructible bc of the private constructor (this may not work in practice,
 * as pybind11 may require that there is a public constructor for py::init)
 */
class LogLevel {
public:
  enum _LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Off,
  };

  static _LogLevel from_string(std::string log_level);
  static std::string to_string(_LogLevel log_level);

private:
  LogLevel() {}
};
