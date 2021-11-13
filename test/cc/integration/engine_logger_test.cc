#include "absl/strings/match.h"
#include "absl/synchronization/notification.h"
#include "gtest/gtest.h"
#include "library/cc/engine_builder.h"

namespace Envoy {
namespace {

TEST(EngineLoggerTest, SetLogger) {
  auto engine_builder = Platform::EngineBuilder();
  absl::Notification startup_log_seen;
  auto engine = engine_builder.addLogLevel(Platform::LogLevel::debug)
                    .setOnEngineRunning([]() {})
                    .setLogger([&](auto msg) {
                      if (absl::StrContains(msg, ("starting main dispatch loop"))) {
                        startup_log_seen.Notify();
                      }
                    })
                    .build();

  startup_log_seen.WaitForNotification();

  engine->terminate();
}

// Verifies that we can start up without specifying a logger.
TEST(EngineLoggerTest, LoggerNotSet) {
  auto engine_builder = Platform::EngineBuilder();
  absl::Notification startup;
  auto engine = engine_builder.addLogLevel(Platform::LogLevel::debug)
                    .setOnEngineRunning([&]() { startup.Notify(); })
                    .build();

  startup.WaitForNotification();

  engine->terminate();
}

} // namespace
} // namespace Envoy
