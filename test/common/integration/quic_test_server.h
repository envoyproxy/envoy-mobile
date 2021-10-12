#pragma once

#include "source/common/stats/isolated_store_impl.h"
#include "source/extensions/transport_sockets/tls/ssl_socket.h"

// test_runner setups
#include "source/common/common/logger.h"
#include "source/exe/process_wide.h"

#include "absl/strings/str_format.h"
#include "envoy/extensions/transport_sockets/quic/v3/quic_transport.pb.h"
#include "test/mocks/server/transport_socket_factory_context.h"
#include "test/integration/autonomous_upstream.h"
#include "test/test_common/network_utility.h"

namespace Envoy {
class QuicTestServer {
private:
  testing::NiceMock<Server::Configuration::MockTransportSocketFactoryContext> factory_context_;
  Stats::IsolatedStoreImpl stats_store_;
  Event::GlobalTimeSystem time_system_;
  Api::ApiPtr api_;
  Network::Address::IpVersion version_;
  std::unique_ptr<AutonomousUpstream> aupstream_;

  void setup();
  Network::TransportSocketFactoryPtr createUpstreamTlsContext(
      testing::NiceMock<Server::Configuration::MockTransportSocketFactoryContext>&);

public:
  QuicTestServer();

  void startQuicTestServer();

  void shutdownQuicTestServer();

  int getServerPort();
};

} // namespace Envoy
