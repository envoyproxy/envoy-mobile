#pragma once

#include "extensions/clusters/dynamic_forward_proxy/cluster.h"
#include "extensions/filters/http/dynamic_forward_proxy/config.h"
#include "extensions/filters/http/router/config.h"
#include "extensions/filters/network/http_connection_manager/config.h"
#include "extensions/stat_sinks/metrics_service/config.h"
#include "extensions/transport_sockets/tls/config.h"

#include "common/upstream/logical_dns_cluster.h"

namespace Envoy {
class ExtensionRegistry {
public:
  static void registerFactories();
};
}
