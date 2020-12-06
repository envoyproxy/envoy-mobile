#pragma once

#include <functional>

class Executor {
public:
  virtual ~Executor();

  virtual void execute(std::function<void()> func) = 0;
};
