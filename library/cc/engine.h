#pragma once

#include "library/common/types/c_types.h"
#include "log_level.h"
#include "stream_client.h"
#include "stats_client.h"

class Engine {
public:
  Engine(envoy_engine_t engine_, const std::string& configuration, LogLevel log_level, std::function<void()> on_engine_running);

  StreamClient& stream_client();
  StatsClient& stats_client();

private:
  StreamClient stream_client_;
  StatsClient stats_client_;
};
