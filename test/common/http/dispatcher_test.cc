#include "common/buffer/buffer_impl.h"
#include "common/http/async_client_impl.h"
#include "common/http/context_impl.h"

#include "test/common/http/common.h"
#include "test/mocks/buffer/mocks.h"
#include "test/mocks/event/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/local_info/mocks.h"
#include "test/mocks/upstream/mocks.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "library/common/buffer/utility.h"
#include "library/common/http/dispatcher.h"
#include "library/common/http/header_utility.h"
#include "library/common/types/c_types.h"

using testing::_;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;
using testing::SaveArg;
using testing::WithArg;

namespace Envoy {
namespace Http {

class DispatcherTest : public testing::Test {
public:
  DispatcherTest()
      : http_context_(stats_store_.symbolTable()),
        client_(cm_.thread_local_cluster_.cluster_.info_, stats_store_, event_dispatcher_,
                local_info_, cm_, runtime_, random_,
                Router::ShadowWriterPtr{new NiceMock<Router::MockShadowWriter>()}, http_context_) {
    http_dispatcher_.ready(event_dispatcher_, cm_);
    ON_CALL(*cm_.conn_pool_.host_, locality())
        .WillByDefault(ReturnRef(envoy::api::v2::core::Locality().default_instance()));
  }

  typedef struct {
    uint32_t on_headers_calls;
    uint32_t on_data_calls;
    uint32_t on_complete_calls;
    uint32_t on_error_calls;
  } callbacks_called;

  Stats::MockIsolatedStatsStore stats_store_;
  MockAsyncClientCallbacks callbacks_;
  MockAsyncClientStreamCallbacks stream_callbacks_;
  NiceMock<Upstream::MockClusterManager> cm_;
  NiceMock<MockStreamEncoder> stream_encoder_;
  StreamDecoder* response_decoder_{};
  NiceMock<Event::MockTimer>* timer_;
  NiceMock<Event::MockDispatcher> event_dispatcher_;
  NiceMock<Runtime::MockLoader> runtime_;
  NiceMock<Runtime::MockRandomGenerator> random_;
  NiceMock<LocalInfo::MockLocalInfo> local_info_;
  Http::ContextImpl http_context_;
  AsyncClientImpl client_;
  Dispatcher http_dispatcher_;
  envoy_http_callbacks bridge_callbacks_;
  NiceMock<StreamInfo::MockStreamInfo> stream_info_;
};

TEST_F(DispatcherTest, BasicStreamHeadersOnly) {
  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    HeaderMapPtr response_headers = Utility::toInternalHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Grab the response decoder in order to dispatch responses on the stream.
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder& decoder,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        response_decoder_ = &decoder;
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(
          WithArg<0>(Invoke([&](AsyncClient::StreamCallbacks& callbacks) -> AsyncClient::Stream* {
            return client_.start(callbacks, AsyncClient::StreamOptions());
          })));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Send request headers.
  Event::PostCb post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, true));
  post_cb();

  // Decode response headers. decodeHeaders with true will bubble up to onHeaders, which will in
  // turn cause closeRemote. Because closeLocal has already been called, cleanup will happen; hence
  // the second call to isThreadSafe.
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  response_decoder_->decode100ContinueHeaders(
      HeaderMapPtr(new TestHeaderMapImpl{{":status", "100"}}));
  response_decoder_->decodeHeaders(HeaderMapPtr(new TestHeaderMapImpl{{":status", "200"}}), true);

  EXPECT_EQ(
      1UL,
      cm_.thread_local_cluster_.cluster_.info_->stats_store_.counter("upstream_rq_200").value());
  EXPECT_EQ(1UL, cm_.thread_local_cluster_.cluster_.info_->stats_store_
                     .counter("internal.upstream_rq_200")
                     .value());

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_headers_calls, 1);
  ASSERT_EQ(cc.on_complete_calls, 1);
  ASSERT_EQ(cc.on_data_calls, 0);
}

TEST_F(DispatcherTest, BasicStream) {
  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_FALSE(end_stream);
    HeaderMapPtr response_headers = Utility::toInternalHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_data = [](envoy_data c_data, bool end_stream, void* context) -> void {
    ASSERT_TRUE(end_stream);
    ASSERT_EQ(Http::Utility::convertToString(c_data), "response body");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_data_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Grab the response decoder in order to dispatch responses on the stream.
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder& decoder,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        response_decoder_ = &decoder;
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Build body data
  Buffer::OwnedImpl request_data = Buffer::OwnedImpl("request body");
  envoy_data c_data = Buffer::Utility::toBridgeData(request_data);

  // Create a stream.
  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(
          WithArg<0>(Invoke([&](AsyncClient::StreamCallbacks& callbacks) -> AsyncClient::Stream* {
            return client_.start(callbacks, AsyncClient::StreamOptions());
          })));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Send request headers.
  Event::PostCb headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, false);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, false));
  headers_post_cb();

  // Send request data.
  Event::PostCb data_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&data_post_cb));
  http_dispatcher_.sendData(stream, c_data, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeData(BufferStringEqual("request body"), true));
  data_post_cb();

  // Decode response headers and data.
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  response_decoder_->decode100ContinueHeaders(
      HeaderMapPtr(new TestHeaderMapImpl{{":status", "100"}}));
  response_decoder_->decodeHeaders(HeaderMapPtr(new TestHeaderMapImpl{{":status", "200"}}), false);
  Buffer::InstancePtr response_data{new Buffer::OwnedImpl("response body")};
  response_decoder_->decodeData(*response_data, true);

  EXPECT_EQ(
      1UL,
      cm_.thread_local_cluster_.cluster_.info_->stats_store_.counter("upstream_rq_200").value());
  EXPECT_EQ(1UL, cm_.thread_local_cluster_.cluster_.info_->stats_store_
                     .counter("internal.upstream_rq_200")
                     .value());

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_headers_calls, 1);
  ASSERT_EQ(cc.on_data_calls, 1);
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, ResetStream) {
  envoy_stream_t stream = 1;
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_error = [](envoy_error actual_error, void* context) -> void {
    envoy_error expected_error = {ENVOY_STREAM_RESET, envoy_nodata};
    ASSERT_EQ(actual_error.error_code, expected_error.error_code);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(
          WithArg<0>(Invoke([&](AsyncClient::StreamCallbacks& callbacks) -> AsyncClient::Stream* {
            return client_.start(callbacks, AsyncClient::StreamOptions());
          })));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  Event::PostCb post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&post_cb));
  http_dispatcher_.resetStream(stream);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(2).WillRepeatedly(Return(true));
  post_cb();

  // Ensure that the on_error on the bridge_callbacks was called.
  ASSERT_EQ(cc.on_error_calls, 1);
  ASSERT_EQ(cc.on_complete_calls, 0);
}

TEST_F(DispatcherTest, MultipleStreams) {
  envoy_stream_t stream1 = 1;
  envoy_stream_t stream2 = 2;
  // Start stream1.
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    HeaderMapPtr response_headers = Utility::toInternalHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Grab the response decoder in order to dispatch responses on the stream.
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder& decoder,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        response_decoder_ = &decoder;
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(
          WithArg<0>(Invoke([&](AsyncClient::StreamCallbacks& callbacks) -> AsyncClient::Stream* {
            return client_.start(callbacks, AsyncClient::StreamOptions());
          })));
  EXPECT_EQ(http_dispatcher_.startStream(stream1, bridge_callbacks), ENVOY_SUCCESS);

  // Send request headers.
  Event::PostCb post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&post_cb));
  http_dispatcher_.sendHeaders(stream1, c_headers, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, true));
  post_cb();

  // Start stream2.
  // Setup bridge_callbacks to handle the response headers.
  NiceMock<MockStreamEncoder> stream_encoder2;
  StreamDecoder* response_decoder2{};
  envoy_http_callbacks bridge_callbacks2;
  callbacks_called cc2 = {false, 0, false, false};
  bridge_callbacks2.context = &cc2;
  bridge_callbacks2.on_headers = [](envoy_headers c_headers, bool end_stream,
                                    void* context) -> void {
    ASSERT_TRUE(end_stream);
    HeaderMapPtr response_headers = Utility::toInternalHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "503");
    bool* on_headers_called2 = static_cast<bool*>(context);
    *on_headers_called2 = true;
  };
  bridge_callbacks2.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Grab the response decoder in order to dispatch responses on the stream.
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder& decoder,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder2, cm_.conn_pool_.host_, stream_info_);
        response_decoder2 = &decoder;
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers2;
  HttpTestUtility::addDefaultHeaders(headers2);
  envoy_headers c_headers2 = Utility::toBridgeHeaders(headers2);

  // Create a stream.
  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(
          WithArg<0>(Invoke([&](AsyncClient::StreamCallbacks& callbacks) -> AsyncClient::Stream* {
            return client_.start(callbacks, AsyncClient::StreamOptions());
          })));
  EXPECT_CALL(event_dispatcher_, post(_));
  EXPECT_EQ(http_dispatcher_.startStream(stream2, bridge_callbacks2), ENVOY_SUCCESS);

  // Send request headers.
  Event::PostCb post_cb2;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&post_cb2));
  http_dispatcher_.sendHeaders(stream2, c_headers2, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder2, encodeHeaders(_, true));
  post_cb2();

  // Finish stream 2.
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  HeaderMapPtr response_headers2(new TestHeaderMapImpl{{":status", "503"}});
  response_decoder2->decodeHeaders(std::move(response_headers2), true);
  // Ensure that the on_headers on the bridge_callbacks was called.
  ASSERT_EQ(cc2.on_headers_calls, 1);
  ASSERT_EQ(cc2.on_complete_calls, 1);

  // Finish stream 1.
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  HeaderMapPtr response_headers(new TestHeaderMapImpl{{":status", "200"}});
  response_decoder_->decodeHeaders(std::move(response_headers), true);
  ASSERT_EQ(cc.on_headers_calls, 1);
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, LocalResetAfterStreamStart) {
  envoy_stream_t stream = 1;
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0};
  bridge_callbacks.context = &cc;

  bridge_callbacks.on_error = [](envoy_error actual_error, void* context) -> void {
    envoy_error expected_error = {ENVOY_STREAM_RESET, envoy_nodata};
    ASSERT_EQ(actual_error.error_code, expected_error.error_code);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_FALSE(end_stream);
    HeaderMapPtr response_headers = Utility::toInternalHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Grab the response decoder in order to dispatch responses on the stream.
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder& decoder,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        response_decoder_ = &decoder;
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(
          WithArg<0>(Invoke([&](AsyncClient::StreamCallbacks& callbacks) -> AsyncClient::Stream* {
            return client_.start(callbacks, AsyncClient::StreamOptions());
          })));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, false);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, false));
  send_headers_post_cb();

  response_decoder_->decodeHeaders(HeaderMapPtr(new TestHeaderMapImpl{{":status", "200"}}), false);
  // Ensure that the on_headers on the bridge_callbacks was called.
  ASSERT_EQ(cc.on_headers_calls, 1);

  Event::PostCb reset_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&reset_post_cb));
  http_dispatcher_.resetStream(stream);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(2).WillRepeatedly(Return(true));
  reset_post_cb();

  // Ensure that the on_error on the bridge_callbacks was called.
  ASSERT_EQ(cc.on_error_calls, 1);
  ASSERT_EQ(cc.on_complete_calls, 0);
}

TEST_F(DispatcherTest, RemoteResetAfterStreamStart) {
  envoy_stream_t stream = 1;
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0};
  bridge_callbacks.context = &cc;

  bridge_callbacks.on_error = [](envoy_error actual_error, void* context) -> void {
    envoy_error expected_error = {ENVOY_STREAM_RESET, envoy_nodata};
    ASSERT_EQ(actual_error.error_code, expected_error.error_code);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_FALSE(end_stream);
    HeaderMapPtr response_headers = Utility::toInternalHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Grab the response decoder in order to dispatch responses on the stream.
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder& decoder,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        response_decoder_ = &decoder;
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(
          WithArg<0>(Invoke([&](AsyncClient::StreamCallbacks& callbacks) -> AsyncClient::Stream* {
            return client_.start(callbacks, AsyncClient::StreamOptions());
          })));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, false);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(2).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, false));
  send_headers_post_cb();

  response_decoder_->decodeHeaders(HeaderMapPtr(new TestHeaderMapImpl{{":status", "200"}}), false);
  // Ensure that the on_headers on the bridge_callbacks was called.
  ASSERT_EQ(cc.on_headers_calls, 1);

  stream_encoder_.getStream().resetStream(StreamResetReason::RemoteReset);
  // Ensure that the on_error on the bridge_callbacks was called.
  ASSERT_EQ(cc.on_error_calls, 1);
  ASSERT_EQ(cc.on_complete_calls, 0);
}

TEST_F(DispatcherTest, DestroyWithActiveStream) {
  envoy_stream_t stream = 1;
  // Grab the response decoder in order to dispatch responses on the stream.
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder& decoder,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        response_decoder_ = &decoder;
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(Return(client_.start(stream_callbacks_, AsyncClient::StreamOptions())));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks_), ENVOY_SUCCESS);

  // Send request headers.
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, false));
  EXPECT_CALL(stream_encoder_.stream_, resetStream(_));
  EXPECT_CALL(stream_callbacks_, onReset());
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  http_dispatcher_.sendHeaders(stream, c_headers, false);
}

TEST_F(DispatcherTest, ResetInOnHeaders) {
  envoy_stream_t stream = 1;
  // Grab the response decoder in order to dispatch responses on the stream.
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder& decoder,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        response_decoder_ = &decoder;
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(Return(client_.start(stream_callbacks_, AsyncClient::StreamOptions())));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks_), ENVOY_SUCCESS);

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, false);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, false));
  send_headers_post_cb();

  TestHeaderMapImpl expected_headers{{":status", "200"}};
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(event_dispatcher_, post(_));
  EXPECT_CALL(stream_callbacks_, onHeaders_(HeaderMapEqualRef(&expected_headers), false))
      .WillOnce(Invoke([this, stream](HeaderMap&, bool) { http_dispatcher_.resetStream(stream); }));
  EXPECT_CALL(stream_callbacks_, onData(_, _)).Times(0);
  EXPECT_CALL(stream_callbacks_, onReset());

  response_decoder_->decodeHeaders(HeaderMapPtr(new TestHeaderMapImpl{{":status", "200"}}), false);
}

TEST_F(DispatcherTest, StreamTimeout) {
  envoy_stream_t stream = 1;
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder&,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(Return(client_.start(stream_callbacks_, AsyncClient::StreamOptions().setTimeout(
                                                            std::chrono::milliseconds(40)))));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks_), ENVOY_SUCCESS);

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, true));
  timer_ = new NiceMock<Event::MockTimer>(&event_dispatcher_);
  EXPECT_CALL(*timer_, enableTimer(std::chrono::milliseconds(40), _));
  EXPECT_CALL(stream_encoder_.stream_, resetStream(_));

  TestHeaderMapImpl expected_timeout{
      {":status", "504"}, {"content-length", "24"}, {"content-type", "text/plain"}};
  EXPECT_CALL(stream_callbacks_, onHeaders_(HeaderMapEqualRef(&expected_timeout), false));
  EXPECT_CALL(stream_callbacks_, onData(_, true));
  EXPECT_CALL(stream_callbacks_, onComplete());
  send_headers_post_cb();
  timer_->invokeCallback();

  EXPECT_EQ(1UL,
            cm_.thread_local_cluster_.cluster_.info_->stats_store_.counter("upstream_rq_timeout")
                .value());
  EXPECT_EQ(1UL, cm_.conn_pool_.host_->stats().rq_timeout_.value());
  EXPECT_EQ(
      1UL,
      cm_.thread_local_cluster_.cluster_.info_->stats_store_.counter("upstream_rq_504").value());
}

TEST_F(DispatcherTest, StreamTimeoutHeadReply) {
  envoy_stream_t stream = 1;
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder&,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers, "HEAD");
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(Return(client_.start(stream_callbacks_, AsyncClient::StreamOptions().setTimeout(
                                                            std::chrono::milliseconds(40)))));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks_), ENVOY_SUCCESS);

  // Send request headers.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, true));
  timer_ = new NiceMock<Event::MockTimer>(&event_dispatcher_);
  EXPECT_CALL(*timer_, enableTimer(std::chrono::milliseconds(40), _));
  EXPECT_CALL(stream_encoder_.stream_, resetStream(_));

  TestHeaderMapImpl expected_timeout{
      {":status", "504"}, {"content-length", "24"}, {"content-type", "text/plain"}};
  EXPECT_CALL(stream_callbacks_, onHeaders_(HeaderMapEqualRef(&expected_timeout), true));
  EXPECT_CALL(stream_callbacks_, onComplete());
  send_headers_post_cb();
  timer_->invokeCallback();
}

TEST_F(DispatcherTest, DisableTimerWithStream) {
  envoy_stream_t stream = 1;
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder&,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        return nullptr;
      }));

  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers, "HEAD");
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(Return(client_.start(stream_callbacks_, AsyncClient::StreamOptions().setTimeout(
                                                            std::chrono::milliseconds(40)))));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks_), ENVOY_SUCCESS);

  // Send request headers and reset stream.
  Event::PostCb send_headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&send_headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);
  Event::PostCb reset_stream_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&reset_stream_post_cb));
  http_dispatcher_.resetStream(stream);

  EXPECT_CALL(stream_encoder_, encodeHeaders(_, true));
  timer_ = new NiceMock<Event::MockTimer>(&event_dispatcher_);
  EXPECT_CALL(*timer_, enableTimer(std::chrono::milliseconds(40), _));
  EXPECT_CALL(*timer_, disableTimer());
  EXPECT_CALL(stream_encoder_.stream_, resetStream(_));
  EXPECT_CALL(stream_callbacks_, onReset());

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  send_headers_post_cb();
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  reset_stream_post_cb();
}

TEST_F(DispatcherTest, MultipleDataStream) {
  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_FALSE(end_stream);
    HeaderMapPtr response_headers = Utility::toInternalHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_data = [](envoy_data, bool, void* context) -> void {
    // TODO: assert end_stream and contents of c_data for multiple calls of on_data.
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_data_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };

  // Grab the response decoder in order to dispatch responses on the stream.
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder& decoder,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        response_decoder_ = &decoder;
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Build first body data
  Buffer::OwnedImpl request_data = Buffer::OwnedImpl("request body");
  envoy_data c_data = Buffer::Utility::toBridgeData(request_data);

  // Build second body data
  Buffer::OwnedImpl request_data2 = Buffer::OwnedImpl("request body2");
  envoy_data c_data2 = Buffer::Utility::toBridgeData(request_data2);

  // Create a stream.
  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(
          WithArg<0>(Invoke([&](AsyncClient::StreamCallbacks& callbacks) -> AsyncClient::Stream* {
            return client_.start(callbacks, AsyncClient::StreamOptions());
          })));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Send request headers.
  Event::PostCb headers_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&headers_post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, false);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, false));
  headers_post_cb();

  // Send request data.
  Event::PostCb data_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&data_post_cb));
  http_dispatcher_.sendData(stream, c_data, false);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeData(BufferStringEqual("request body"), false));
  data_post_cb();

  // Send second request data.
  Event::PostCb data_post_cb2;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&data_post_cb2));
  http_dispatcher_.sendData(stream, c_data2, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeData(BufferStringEqual("request body2"), true));
  data_post_cb2();

  // Decode response headers and data.
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  response_decoder_->decode100ContinueHeaders(
      HeaderMapPtr(new TestHeaderMapImpl{{":status", "100"}}));
  response_decoder_->decodeHeaders(HeaderMapPtr(new TestHeaderMapImpl{{":status", "200"}}), false);
  Buffer::InstancePtr response_data{new Buffer::OwnedImpl("response body")};
  response_decoder_->decodeData(*response_data, false);
  Buffer::InstancePtr response_data2{new Buffer::OwnedImpl("response body2")};
  response_decoder_->decodeData(*response_data2, true);

  EXPECT_EQ(
      1UL,
      cm_.thread_local_cluster_.cluster_.info_->stats_store_.counter("upstream_rq_200").value());
  EXPECT_EQ(1UL, cm_.thread_local_cluster_.cluster_.info_->stats_store_
                     .counter("internal.upstream_rq_200")
                     .value());

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_headers_calls, 1);
  ASSERT_EQ(cc.on_data_calls, 2);
  ASSERT_EQ(cc.on_complete_calls, 1);
}

TEST_F(DispatcherTest, StreamResetAfterOnComplete) {
  envoy_stream_t stream = 1;
  // Setup bridge_callbacks to handle the response headers.
  envoy_http_callbacks bridge_callbacks;
  callbacks_called cc = {0, 0, 0, 0};
  bridge_callbacks.context = &cc;
  bridge_callbacks.on_headers = [](envoy_headers c_headers, bool end_stream,
                                   void* context) -> void {
    ASSERT_TRUE(end_stream);
    HeaderMapPtr response_headers = Utility::toInternalHeaders(c_headers);
    EXPECT_EQ(response_headers->Status()->value().getStringView(), "200");
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_headers_calls++;
  };
  bridge_callbacks.on_complete = [](void* context) -> void {
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_complete_calls++;
  };
  bridge_callbacks.on_error = [](envoy_error actual_error, void* context) -> void {
    envoy_error expected_error = {ENVOY_STREAM_RESET, envoy_nodata};
    ASSERT_EQ(actual_error.error_code, expected_error.error_code);
    callbacks_called* cc = static_cast<callbacks_called*>(context);
    cc->on_error_calls++;
  };

  // Grab the response decoder in order to dispatch responses on the stream.
  EXPECT_CALL(cm_.conn_pool_, newStream(_, _))
      .WillOnce(Invoke([&](StreamDecoder& decoder,
                           ConnectionPool::Callbacks& callbacks) -> ConnectionPool::Cancellable* {
        callbacks.onPoolReady(stream_encoder_, cm_.conn_pool_.host_, stream_info_);
        response_decoder_ = &decoder;
        return nullptr;
      }));

  // Build a set of request headers.
  TestHeaderMapImpl headers;
  HttpTestUtility::addDefaultHeaders(headers);
  envoy_headers c_headers = Utility::toBridgeHeaders(headers);

  // Create a stream.
  EXPECT_CALL(cm_, httpAsyncClientForCluster("base")).WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _))
      .WillOnce(
          WithArg<0>(Invoke([&](AsyncClient::StreamCallbacks& callbacks) -> AsyncClient::Stream* {
            return client_.start(callbacks, AsyncClient::StreamOptions());
          })));
  EXPECT_EQ(http_dispatcher_.startStream(stream, bridge_callbacks), ENVOY_SUCCESS);

  // Send request headers.
  Event::PostCb post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&post_cb));
  http_dispatcher_.sendHeaders(stream, c_headers, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(stream_encoder_, encodeHeaders(_, true));
  post_cb();

  // Decode response headers. decodeHeaders with true will bubble up to onHeaders, which will in
  // turn cause closeRemote. Because closeLocal has already been called, cleanup will happen; hence
  // the second call to isThreadSafe.
  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  response_decoder_->decode100ContinueHeaders(
      HeaderMapPtr(new TestHeaderMapImpl{{":status", "100"}}));
  response_decoder_->decodeHeaders(HeaderMapPtr(new TestHeaderMapImpl{{":status", "200"}}), true);

  EXPECT_EQ(
      1UL,
      cm_.thread_local_cluster_.cluster_.info_->stats_store_.counter("upstream_rq_200").value());
  EXPECT_EQ(1UL, cm_.thread_local_cluster_.cluster_.info_->stats_store_
                     .counter("internal.upstream_rq_200")
                     .value());

  // resetStream after onComplete has fired is a no-op, as the stream is cleaned from the
  // dispatcher.
  Event::PostCb reset_post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&reset_post_cb));
  http_dispatcher_.resetStream(stream);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(1).WillRepeatedly(Return(true));
  reset_post_cb();

  // Ensure that the callbacks on the bridge_callbacks were called.
  ASSERT_EQ(cc.on_headers_calls, 1);
  ASSERT_EQ(cc.on_complete_calls, 1);
  ASSERT_EQ(cc.on_data_calls, 0);
  ASSERT_EQ(cc.on_error_calls, 0);
}

} // namespace Http
} // namespace Envoy
