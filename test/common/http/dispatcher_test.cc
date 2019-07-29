#include "common/http/async_client_impl.h"
#include "common/http/context_impl.h"

#include "test/common/http/common.h"
#include "test/mocks/event/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/local_info/mocks.h"
#include "test/mocks/upstream/mocks.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "library/common/http/dispatcher.h"
#include "library/common/http/header_utility.h"
#include "library/common/include/c_types.h"

using testing::_;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;
using testing::SaveArg;
using testing::WithArg;

namespace Envoy {
namespace Http {

envoy_data envoyString(std::string& s) {
  return {s.size(), reinterpret_cast<const uint8_t*>(s.c_str())};
}

class DispatcherTest : public testing::Test {
public:
  DispatcherTest()
      : http_context_(stats_store_.symbolTable()),
        client_(cm_.thread_local_cluster_.cluster_.info_, stats_store_, event_dispatcher_,
                local_info_, cm_, runtime_, random_,
                Router::ShadowWriterPtr{new NiceMock<Router::MockShadowWriter>()}, http_context_),
        http_dispatcher_(event_dispatcher_, cm_) {
    ON_CALL(*cm_.conn_pool_.host_, locality())
        .WillByDefault(ReturnRef(envoy::api::v2::core::Locality().default_instance()));
  }

  Stats::MockIsolatedStatsStore stats_store_;
  MockAsyncClientCallbacks callbacks_;
  NiceMock<Upstream::MockClusterManager> cm_;
  NiceMock<Event::MockDispatcher> event_dispatcher_;
  NiceMock<Runtime::MockLoader> runtime_;
  NiceMock<Runtime::MockRandomGenerator> random_;
  NiceMock<LocalInfo::MockLocalInfo> local_info_;
  Http::ContextImpl http_context_;
  AsyncClientImpl client_;
  Dispatcher http_dispatcher_;
  envoy_observer observer_;
};

TEST_F(DispatcherTest, SendHeaders) {
  std::vector<std::pair<std::string, std::string>> header_strings = {
      {":method", "GET"}, {":scheme", "https"}, {":authority", "api.lyft.com"}, {":path", "/ping"}};

  envoy_header* header_array = new envoy_header[header_strings.size()];

  for (size_t i = 0; i < header_strings.size(); i++) {
    header_array[i] = {
        envoyString(header_strings[i].first),
        envoyString(header_strings[i].second),
    };
  }

  envoy_headers headers = {header_strings.size(), header_array};
  NiceMock<MockAsyncClientStream> underlying_stream;

  EXPECT_CALL(cm_, httpAsyncClientForCluster("egress_cluster"))
      .WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _)).WillOnce(Return(&underlying_stream));
  envoy_stream_t stream_id = http_dispatcher_.startStream(observer_);

  Event::PostCb post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&post_cb));
  http_dispatcher_.sendHeaders(stream_id, headers, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(2).WillRepeatedly(Return(true));
  EXPECT_CALL(underlying_stream, sendHeaders(_, true)).Times(1);
  post_cb();
}

TEST_F(DispatcherTest, SendHeadersNoStream) {
  envoy_headers headers;
  Event::PostCb post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&post_cb));
  http_dispatcher_.sendHeaders(0, headers, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  // FIXME better proof that sendHeaders was not called.
  post_cb();
}

TEST_F(DispatcherTest, ResetStream) {
  envoy_observer observer;

  observer.on_error_f = [](envoy_error actual_error, void*) -> void {
    envoy_error expected_error = {ENVOY_STREAM_RESET, {0, nullptr}};
    ASSERT_EQ(actual_error.error_code, expected_error.error_code);
  };

  EXPECT_CALL(cm_, httpAsyncClientForCluster("egress_cluster"))
      .WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(
          WithArg<0>(Invoke([&](AsyncClient::StreamCallbacks& callbacks) -> AsyncClient::Stream* {
            return client_.start(callbacks, AsyncClient::StreamOptions());
          })));
  envoy_stream_t stream_id = http_dispatcher_.startStream(observer);

  Event::PostCb post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&post_cb));
  http_dispatcher_.resetStream(stream_id);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(4).WillRepeatedly(Return(true));
  post_cb();
}

} // namespace Http
} // namespace Envoy
