#include "library/common/envoy_mobile_main_common.h"

#include "common/runtime/runtime_impl.h"

namespace Envoy {

MobileMainCommon::MobileMainCommon(int argc, const char* const* argv)
    : options_(argc, argv, &MainCommon::hotRestartVersion, spdlog::level::info),
      base_(options_, real_time_system_, default_listener_hooks_, prod_component_factory_,
            std::make_unique<Runtime::RandomGeneratorImpl>(), platform_impl_.threadFactory(),
            platform_impl_.fileSystem(), nullptr) {
  options_.setSignalHandling(false);
}

} // namespace Envoy
