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
  MockAsyncClientStreamCallbacks stream_callbacks_;
  NiceMock<Upstream::MockClusterManager> cm_;
  NiceMock<MockStreamEncoder> stream_encoder_;
  StreamDecoder* response_decoder_{};
  NiceMock<Event::MockDispatcher> event_dispatcher_;
  NiceMock<Runtime::MockLoader> runtime_;
  NiceMock<Runtime::MockRandomGenerator> random_;
  NiceMock<LocalInfo::MockLocalInfo> local_info_;
  Http::ContextImpl http_context_;
  AsyncClientImpl client_;
  Dispatcher http_dispatcher_;
  envoy_observer observer_;
};

TEST_F(DispatcherTest, BasicStreamHeadersOnly) {
  // Setup observer to handle the response headers.
  envoy_observer observer;
  bool on_headers_called = false;
  observer.context = &on_headers_called;
  observer.on_headers_f = [](envoy_headers c_headers, bool end_stream, void* context) -> void {
    ASSERT_TRUE(end_stream);
    HeaderMapPtr response_headers = Utility::transformHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    bool* on_headers_called = static_cast<bool*>(context);
    *on_headers_called = true;
  };

  // Grab the response decoder in order to dispatch responses on the stream.
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder& decoder,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_);
        response_decoder_ = &decoder;
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::transformHeaders(headers);

  // Create a stream.
  EXPECT_CALL(cm_, httpAsyncClientForCluster("egress_cluster"))
      .WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(
          WithArg<0>(Invoke([&](AsyncClient::StreamCallbacks& callbacks) -> AsyncClient::Stream* {
            return client_.start(callbacks, AsyncClient::StreamOptions());
          })));
  envoy_stream_t stream_id = http_dispatcher_.startStream(observer);

  // Send request headers.
  Event::PostCb post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&post_cb));
  http_dispatcher_.sendHeaders(stream_id, c_headers, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(2).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, true));
  post_cb();

  // Decode response headers. decodeHeaders with true will bubble up to onHeaders, which will in
  // turn cause closeRemote. Because closeLocal has already been called, cleanup will happen; hence
  // the second call to isThreadSafe.
  // TODO: find a way to make the fact that the stream is correctly closed more explicit.
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(2).WillRepeatedly(Return(true));
  response_decoder_->decode100ContinueHeaders(
      HeaderMapPtr(new TestHeaderMapImpl{{":status", "100"}}));
  response_decoder_->decodeHeaders(HeaderMapPtr(new TestHeaderMapImpl{{":status", "200"}}), true);

  EXPECT_EQ(
      1UL,
      cm_.thread_local_cluster_.cluster_.info_->stats_store_.counter("upstream_rq_200").value());
  EXPECT_EQ(1UL, cm_.thread_local_cluster_.cluster_.info_->stats_store_
                     .counter("internal.upstream_rq_200")
                     .value());

  // Ensure that the on_headers_f on the observer was called.
  ASSERT_TRUE(on_headers_called);
}

TEST_F(DispatcherTest, ResetStream) {
  envoy_observer observer;
  bool on_error_called = false;
  observer.context = &on_error_called;
  observer.on_error_f = [](envoy_error actual_error, void* context) -> void {
    envoy_error expected_error = {ENVOY_STREAM_RESET, {0, nullptr}};
    ASSERT_EQ(actual_error.error_code, expected_error.error_code);
    bool* on_error_called = static_cast<bool*>(context);
    *on_error_called = true;
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

  // Ensure that the on_error_f on the observer was called.
  ASSERT_TRUE(on_error_called);
}

} // namespace Http
} // namespace Envoy
