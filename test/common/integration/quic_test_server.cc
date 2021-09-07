#include "quic_test_server.h"

using bazel::tools::cpp::runfiles::Runfiles;
namespace Envoy {

  // see https://github.com/envoyproxy/envoy/blob/main/test/test_runner.cc
  void QuicTestServer::setup(int argc, char** argv) {
    Envoy::TestEnvironment::initializeTestMain(argv[0]);

    std::string error;
    const std::basic_string<char> argv0;
    std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
    RELEASE_ASSERT(Envoy::TestEnvironment::getOptionalEnvVar("NORUNFILES").has_value() ||
        runfiles != nullptr,
                   error);

    Envoy::TestEnvironment::setRunfiles(runfiles.get());

    Envoy::TestEnvironment::setEnvVar("ENVOY_IP_TEST_VERSIONS", "v4only", 0);

    ::testing::InitGoogleMock(&argc, argv);

    ProcessWide process_wide;

    ::testing::FLAGS_gtest_death_test_style = "threadsafe";

    testing::Test::RecordProperty("TemporaryDirectory", TestEnvironment::temporaryDirectory());

    TestEnvironment::setEnvVar("TEST_UDSDIR", TestEnvironment::unixDomainSocketDirectory(), 1);

    TestEnvironment::initializeOptions(argc, argv);
    Thread::MutexBasicLockable lock;

    Server::Options& options = TestEnvironment::getOptions();

    Logger::Registry::getSink()->setLock(lock);
    Logger::Registry::getSink()->setShouldEscape(false);
    Logger::Registry::setLogLevel(options.logLevel());
    Logger::Registry::setLogFormat(options.logFormat());

    // TODO(colibie) this doesnt work. Why?
    // Logger::Context logging_state(options.logLevel(), options.logFormat(), lock, false,
    //                               options.enableFineGrainLogging());

    if (Logger::Registry::getSink()->hasLock()) {std::cerr << "true \n";};
    // Allocate fake log access manager.
    testing::NiceMock<AccessLog::MockAccessLogManager> access_log_manager;
    std::unique_ptr<Logger::FileSinkDelegate> file_logger;

    // Redirect all logs to fake file when --log-path arg is specified in command line.
    if (!TestEnvironment::getOptions().logPath().empty()) {
      file_logger = std::make_unique<Logger::FileSinkDelegate>(
          TestEnvironment::getOptions().logPath(), access_log_manager, Logger::Registry::getSink());
    }

  }

  Network::TransportSocketFactoryPtr QuicTestServer::createUpstreamTlsContext(testing::NiceMock<Server::Configuration::MockTransportSocketFactoryContext>& factory_context) {
    envoy::extensions::transport_sockets::tls::v3::DownstreamTlsContext tls_context;
    Extensions::TransportSockets::Tls::ContextManagerImpl context_manager_{time_system_};
    std::cerr << "quic_test_serverL50\n";
    //TODO (colibie) fix runfilesPath error; std::cerr << TestEnvironment::runfilesPath("") << " testpath\n";
    const std::string yaml = absl::StrFormat(
        R"EOF(
common_tls_context:
  tls_certificates:
  - certificate_chain: { filename: "/usr/local/google/home/colibie/.cache/bazel/_bazel_colibie/bdfb488ae034f37da2dcebdbd6e4d897/execroot/envoy_mobile/bazel-out/k8-fastbuild/bin/test/java/org/chromium/net/quic_test.runfiles/envoy/test/config/integration/certs/upstreamcert.pem" }
    private_key: { filename: "/usr/local/google/home/colibie/.cache/bazel/_bazel_colibie/bdfb488ae034f37da2dcebdbd6e4d897/execroot/envoy_mobile/bazel-out/k8-fastbuild/bin/test/java/org/chromium/net/quic_test.runfiles/envoy/test/config/integration/certs/upstreamkey.pem" }
  validation_context:
    trusted_ca: { filename: "/usr/local/google/home/colibie/.cache/bazel/_bazel_colibie/bdfb488ae034f37da2dcebdbd6e4d897/execroot/envoy_mobile/bazel-out/k8-fastbuild/bin/test/java/org/chromium/net/quic_test.runfiles/envoy/test/config/integration/certs/cacert.pem" }
)EOF");
    TestUtility::loadFromYaml(yaml, tls_context);
    envoy::extensions::transport_sockets::quic::v3::QuicDownstreamTransport quic_config;
    quic_config.mutable_downstream_tls_context()->MergeFrom(tls_context);

    std::vector<std::string> server_names;
    auto& config_factory = Config::Utility::getAndCheckFactoryByName<
        Server::Configuration::DownstreamTransportSocketConfigFactory>(
        "envoy.transport_sockets.quic");
    std::cerr << "quic_test_serverL73\n";
    return config_factory.createTransportSocketFactory(quic_config, factory_context, server_names);
  }

  QuicTestServer::QuicTestServer() : api_(Api::createApiForTest(stats_store_, time_system_)),
                    version_(Network::Address::IpVersion::v4),
                    config_helper_(version_, *api_, ConfigHelper::baseConfig() + R"EOF(
    filter_chains:
      filters:
    )EOF"){
    ON_CALL(factory_context_, api()).WillByDefault(testing::ReturnRef(*api_));
    ON_CALL(factory_context_, scope()).WillByDefault(testing::ReturnRef(stats_store_));

    // TODO(colibie) remove test param value
    char param[] = "/usr/local/google/home/colibie/.cache/bazel/_bazel_colibie/bdfb488ae034f37da2dcebdbd6e4d897/execroot/envoy_mobile/external/envoy/";

    char *argv[]{param, NULL};
    setup(1, argv);
  }

  void QuicTestServer::startQuicTestServer() {
    FakeUpstreamConfig upstream_config_{time_system_};
    upstream_config_.upstream_protocol_ = Http::CodecType::HTTP3;
    upstream_config_.udp_fake_upstream_ = FakeUpstreamConfig::UdpConfig();

    Network::TransportSocketFactoryPtr factory = createUpstreamTlsContext(factory_context_); // Network::Test::createRawBufferSocketFactory();

    int port = 0;  // let the kernel pick a port that is not in use (avoids test races)
    aupstream = std::make_unique<AutonomousUpstream>(std::move(factory), port, version_, upstream_config_, false);

    // see what port was selected.
    std::cerr << "Upstream now listening on " << aupstream->localAddress()->asString();

    Logger::Registry::getSink()->clearLock();
  }

  void QuicTestServer::shutdownQuicTestServer() {
    aupstream.reset();
//    FAIL() << "this way blaze will give you a test log";
  }

  int QuicTestServer::getServerPort() {
    std::cerr << "quic_test_serverL147\n";
    return aupstream->localAddress()->ip()->port();
  }
} // namespace Envoy

