#pragma once

#include "stream_prototype.h"

class StreamClient {
public:
  virtual ~StreamClient() {}

  virtual StreamPrototype new_stream_prototype() = 0;
};
