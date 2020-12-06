#pragma once

#include "stream_client.h"
#include "stats_client.h"

class Engine {
public:
  virtual ~Engine() {}

  virtual StreamClient stream_client() = 0;
  virtual StatsClient stats_client() = 0;
};
