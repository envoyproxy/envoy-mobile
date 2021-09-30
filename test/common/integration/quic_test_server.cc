#include "quic_test_server.h"

namespace Envoy {

Network::TransportSocketFactoryPtr QuicTestServer::createUpstreamTlsContext(
    testing::NiceMock<Server::Configuration::MockTransportSocketFactoryContext>& factory_context) {
  envoy::extensions::transport_sockets::tls::v3::DownstreamTlsContext tls_context;
  Extensions::TransportSockets::Tls::ContextManagerImpl context_manager_{time_system_};

  const std::string yaml = absl::StrFormat(
      R"EOF(
common_tls_context:
  alpn_protocols: h3
  tls_certificates:
  - certificate_chain:
      inline_string: |
        -----BEGIN CERTIFICATE-----
        MIIEPjCCAyagAwIBAgIUEuy1WgSCzX6mojPirk7Th6uhNHowDQYJKoZIhvcNAQEL
        BQAwfzELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExFjAUBgNVBAcM
        DVNhbiBGcmFuY2lzY28xDTALBgNVBAoMBEx5ZnQxGTAXBgNVBAsMEEx5ZnQgRW5n
        aW5lZXJpbmcxGTAXBgNVBAMMEFRlc3QgVXBzdHJlYW0gQ0EwHhcNMjAwODA1MTkx
        NjAyWhcNMjIwODA1MTkxNjAyWjCBgzELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNh
        bGlmb3JuaWExFjAUBgNVBAcMDVNhbiBGcmFuY2lzY28xDTALBgNVBAoMBEx5ZnQx
        GTAXBgNVBAsMEEx5ZnQgRW5naW5lZXJpbmcxHTAbBgNVBAMMFFRlc3QgVXBzdHJl
        YW0gU2VydmVyMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtpiYA4/I
        NuflkPe4L/GTslmngNQUCo8TzPXG0gt7uoxr4FeuVy7AaD28S2/hwhbl+bDtHTQY
        mvBUwNMsYzpND2eQ3sSIumdeLzBEKP2mnnZ9gE/Zd2TIuZl686RpDq0B6ZdZSpCu
        bqQmmPFLiRNH8JViJZMN5yqMt7T5oq+DnCYQZllqmpAwd6NnhKALrYmZ87oqc0zh
        kf+5amP7zMYKkwQuRwcx4QPZkEp3+qhszolpAJ52dFGJ+pLuUVDg0Gf0cnxLjFKc
        6vcTlj4tsymR4ci58MHRt4EdGdhShw0oaj67gRRfU4Vj61I2ZAVH07kL0mjO2TZT
        EKrOEJJ7/dtxdwIDAQABo4GsMIGpMAwGA1UdEwEB/wQCMAAwCwYDVR0PBAQDAgXg
        MB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATAtBgNVHREEJjAkggoqLmx5
        ZnQuY29thwR/AAABhxAAAAAAAAAAAAAAAAAAAAABMB0GA1UdDgQWBBQeoC5wxwX5
        k3ggIN/844/6jKx9czAfBgNVHSMEGDAWgBQ7Zh1TopMm7SY1RCOEO8L1G8IAZDAN
        BgkqhkiG9w0BAQsFAAOCAQEA18wEg8LnPm99cIouFUFMAO+BpiY2KVa9Bu6x07m9
        quNFv7/4mLt87sk/umD3LH/tDjqW0D84vhG9a+0yDq7ZrD/P5eK3R+yBINwhe4/x
        obJlThEsbcZF1FkMnq1rt53izukyQLLQwoVxidQl3HCg3hosWmpH1VBPgwoize6V
        aAhKLW0n+JSfIE1d80nvZdYlHuCnS6UhLmAbTBCnwT0aGTfzT0Dd4KlYiY8vGZRu
        tXOw4MzKtJcOL3t7Zpz2mhqN25dyiuyvKEhLXdx48aemwa2t6ISfFKsd0/glnNe/
        PFZMakzKv1G0xLGURjsInCZ0kePAmerfZN6CBZDo4laYEg==
        -----END CERTIFICATE-----
    private_key:
      inline_string: |
        -----BEGIN RSA PRIVATE KEY-----
        MIIEpAIBAAKCAQEAtpiYA4/INuflkPe4L/GTslmngNQUCo8TzPXG0gt7uoxr4Feu
        Vy7AaD28S2/hwhbl+bDtHTQYmvBUwNMsYzpND2eQ3sSIumdeLzBEKP2mnnZ9gE/Z
        d2TIuZl686RpDq0B6ZdZSpCubqQmmPFLiRNH8JViJZMN5yqMt7T5oq+DnCYQZllq
        mpAwd6NnhKALrYmZ87oqc0zhkf+5amP7zMYKkwQuRwcx4QPZkEp3+qhszolpAJ52
        dFGJ+pLuUVDg0Gf0cnxLjFKc6vcTlj4tsymR4ci58MHRt4EdGdhShw0oaj67gRRf
        U4Vj61I2ZAVH07kL0mjO2TZTEKrOEJJ7/dtxdwIDAQABAoIBACz6E1+1N/0GTA7U
        ZgMxP09MNC1QkAs1yQvQcoPknjqKQjxFfMUu1+gVZN80FOjpGQbTJOTvoyvvDQFe
        Qu3CO58SxKWKxZ8cvR9khTWPnU4lI67KfGejZKoK+zUuh049IV53kGAEmWLZfkRo
        E1IVdL/3G/DjcyZA3d6WbnM7RnDcqORPnig4lq5HxN76eBdssbxtrAi3Sjy3ChMy
        BLInnryF4UtaT5xqR26YjgtFmYkunrgXTe1i/ewQgBBkSPXcNr7or69hCCv0SG9e
        vRsv1r+Uug3/iRZDjEhKBmXWNAZJ/IsDF37ywiyeBdUY+klDX+mWz+0BB0us8b4u
        LxoZQTECgYEA2Gu9EVC0RMrQ9FF5AgKKJWmZKkOn346RkPrtbl5lbuUgnVdBXJjr
        wfMZVTD/8E/tMN4EMSGmC9bxCpRRzhrphrm7SHGD6b9O30DH9q0TV0r0A8IG/bMO
        xJLYjrYVxtEE+KckzvyvfIefbDG7wYkI3u+ObmjBg9t6jcErKlI/PdkCgYEA1/1E
        T+cpR16iOPz1mz+f/GU4YmPkdSDj/PrjMv0c1OTDvdPiZPpLkhLUKiICnKSKbYjX
        Ko8fdZc3cmakgD1dXtAfR7Tf/hXQIR5+iHD48I5e9DVlkqMNDObfj82ohTFKVe/P
        ZSwgDiAPTMFxWr26u/GzY5D3adCQYJyKE2wTh88CgYEAu7vpzGVnmu0ciXNLNvUh
        BQcvODxsGT9BArTI1Z7I+oOD4TjZmAuHJz1L0lypB7stk+BjXoND2K1hdr3moJUz
        0gy3a0YdGd07++nkDBVi26xHNCNRkS2MN/TyKgnFpiuW1mOXSH5lc+7p2h7iMiY/
        LbQ8p4Xzp//xtZnFafbiqTECgYEAwDN5KZ1r5z24H/xCVv+cT46HSU7ZCr3VA9cC
        fOouUOitouu9J9xviTJGKKQRLPFi2awOxKmN9ic1SRE7y35P60JKw5WaSdGBXydy
        s9nMPMyEhM5Lb9y2jUeZo68ACl5dZvG63a4RbGBtHQF67KOvWvXvi2eCM2BMShyi
        5jujeZMCgYAjewq1hVqL1FOD8sIFpmndsH3+Dfc7BJ/erqGOX9bQYGvJO4nCe+7K
        4o8qFQf4jwdxu0iNxYJIMdn+l4/pz2e7GUFHjgMduUclf27Qj1p+8EyYqp6cmkzM
        8mcwRkYo3aM70EmUu0Xxi3d5O5F1bIJ5MkgXaX/zSF2N02B3jXroxQ==
        -----END RSA PRIVATE KEY-----
  validation_context:
    trusted_ca:
      inline_string: |
        -----BEGIN CERTIFICATE-----
        MIID3TCCAsWgAwIBAgIUdCu/mLip3X/We37vh3BA9u/nxakwDQYJKoZIhvcNAQEL
        BQAwdjELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExFjAUBgNVBAcM
        DVNhbiBGcmFuY2lzY28xDTALBgNVBAoMBEx5ZnQxGTAXBgNVBAsMEEx5ZnQgRW5n
        aW5lZXJpbmcxEDAOBgNVBAMMB1Rlc3QgQ0EwHhcNMjAwODA1MTkxNjAwWhcNMjIw
        ODA1MTkxNjAwWjB2MQswCQYDVQQGEwJVUzETMBEGA1UECAwKQ2FsaWZvcm5pYTEW
        MBQGA1UEBwwNU2FuIEZyYW5jaXNjbzENMAsGA1UECgwETHlmdDEZMBcGA1UECwwQ
        THlmdCBFbmdpbmVlcmluZzEQMA4GA1UEAwwHVGVzdCBDQTCCASIwDQYJKoZIhvcN
        AQEBBQADggEPADCCAQoCggEBALu2Ihi4DmaQG7zySZlWyM9SjxOXCI5840V7Hn0C
        XoiI8sQQmKSC2YCzsaphQoJ0lXCi6Y47o5FkooYyLeNDQTGS0nh+IWm5RCyochtO
        fnaKPv/hYxhpyFQEwkJkbF1Zt1s6j2rq5MzmbWZx090uXZEE82DNZ9QJaMPu6VWt
        iwGoGoS5HF5HNlUVxLNUsklNH0ZfDafR7/LC2ty1vO1c6EJ6yCGiyJZZ7Ilbz27Q
        HPAUd8CcDNKCHZDoMWkLSLN3Nj1MvPVZ5HDsHiNHXthP+zV8FQtloAuZ8Srsmlyg
        rJREkc7gF3f6HrH5ShNhsRFFc53NUjDbYZuha1u4hiOE8lcCAwEAAaNjMGEwDwYD
        VR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFJZL2ixTtL6V
        xpNz4qekny4NchiHMB8GA1UdIwQYMBaAFJZL2ixTtL6VxpNz4qekny4NchiHMA0G
        CSqGSIb3DQEBCwUAA4IBAQAcgG+AaCdrUFEVJDn9UsO7zqzQ3c1VOp+WAtAU8OQK
        Oc4vJYVVKpDs8OZFxmukCeqm1gz2zDeH7TfgCs5UnLtkplx1YO1bd9qvserJVHiD
        LAK+Yl24ZEbrHPaq0zI1RLchqYUOGWmi51pcXi1gsfc8DQ3GqIXoai6kYJeV3jFJ
        jxpQSR32nx6oNN/6kVKlgmBjlWrOy7JyDXGim6Z97TzmS6Clctewmw/5gZ9g+M8e
        g0ZdFbFkNUjzSNm44hiDX8nR6yJRn+gLaARaJvp1dnT+MlvofZuER17WYKH4OyMs
        ie3qKR3an4KC20CtFbpZfv540BVuTTOCtQ5xqZ/LTE78
        -----END CERTIFICATE-----
)EOF");
  TestUtility::loadFromYaml(yaml, tls_context);
  envoy::extensions::transport_sockets::quic::v3::QuicDownstreamTransport quic_config;
  quic_config.mutable_downstream_tls_context()->MergeFrom(tls_context);

  std::vector<std::string> server_names;
  auto& config_factory = Config::Utility::getAndCheckFactoryByName<
      Server::Configuration::DownstreamTransportSocketConfigFactory>(
      "envoy.transport_sockets.quic");

  return config_factory.createTransportSocketFactory(quic_config, factory_context, server_names);
}

QuicTestServer::QuicTestServer()
    : api_(Api::createApiForTest(stats_store_, time_system_)),
      version_(Network::Address::IpVersion::v4),
      config_helper_(version_, *api_, ConfigHelper::baseConfig() + R"EOF(
    filter_chains:
      filters:
      - name: envoy.filters.network.http_connection_manager
        typed_config:
          "@type": type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager
          codec_type: HTTP3
          stat_prefix: remote_hcm
          route_config:
            name: remote_route
            virtual_hosts:
            - name: remote_service
              domains: ["*"]
              routes:
              - match: { prefix: "/" }
                direct_response: { status: 200 }
          http3_protocol_options:
          http_filters:
          - name: envoy.router
            typed_config:
              "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router
    )EOF") {
  ON_CALL(factory_context_, api()).WillByDefault(testing::ReturnRef(*api_));
  ON_CALL(factory_context_, scope()).WillByDefault(testing::ReturnRef(stats_store_));

}

void QuicTestServer::startQuicTestServer() {
  // pre-setup: see https://github.com/envoyproxy/envoy/blob/main/test/test_runner.cc
  ProcessWide process_wide;
  Thread::MutexBasicLockable lock;
  Logger::Context logging_state(spdlog::level::level_enum::err, "[%Y-%m-%d %T.%e][%t][%l][%n] [%g:%#] %v", lock, false,
                                   false);
  // end pre-setup

  FakeUpstreamConfig upstream_config_{time_system_};
  upstream_config_.upstream_protocol_ = Http::CodecType::HTTP3;
  upstream_config_.udp_fake_upstream_ = FakeUpstreamConfig::UdpConfig();

  Network::TransportSocketFactoryPtr factory = createUpstreamTlsContext(factory_context_);
      // Network::Test::createRawBufferSocketFactory();

  int port = 0; // let the kernel pick a port that is not in use (avoids test races)
  aupstream = std::make_unique<AutonomousUpstream>(std::move(factory), port, version_,
                                                   upstream_config_, true);

  aupstream->setLastRequestHeaders(Http::TestRequestHeaderMapImpl{
                                            {"response_size_bytes", "2"}});
  aupstream->setResponseHeaders(std::make_unique<Http::TestResponseHeaderMapImpl>(
                                           Http::TestResponseHeaderMapImpl({{":status", "203"}})));

  // see what port was selected.
  std::cerr << "Upstream now listening on " << aupstream->localAddress()->asString() << "\n";

}

void QuicTestServer::shutdownQuicTestServer() {
  aupstream.reset();
  //    FAIL() << "this way blaze will give you a test log";
}

int QuicTestServer::getServerPort() { return aupstream->localAddress()->ip()->port(); }
} // namespace Envoy
