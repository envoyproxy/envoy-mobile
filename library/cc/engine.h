#pragma once

#include "library/common/types/c_types.h"
#include "log_level.h"
#include "stats_client.h"
#include "stream_client.h"

class Engine {
public:
  Engine(envoy_engine_t engine_, const std::string& configuration, LogLevel log_level, std::function<void()> on_engine_running);

  StreamClientSharedPtr stream_client();
  StatsClientSharedPtr stats_client();

private:
  StreamClientSharedPtr stream_client_;
  StatsClientSharedPtr stats_client_;
};

using EngineSharedPtr = std::shared_ptr<Engine>;
