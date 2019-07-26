#include "test/mocks/event/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/upstream/mocks.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "library/common/http/dispatcher.h"
#include "library/common/include/c_types.h"

using testing::_;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;
using testing::SaveArg;

namespace Envoy {
namespace Http {

envoy_data envoyString(std::string& s) {
  return {s.size(), reinterpret_cast<const uint8_t*>(s.c_str())};
}

class DispatcherTest : public testing::Test {
public:
  DispatcherTest() : http_dispatcher_(event_dispatcher_, cm_) {
    observer_.on_headers_f = [](envoy_headers, bool, void*) -> void { ASSERT_EQ(1, 0); };
  }

  NiceMock<Upstream::MockClusterManager> cm_;
  NiceMock<Event::MockDispatcher> event_dispatcher_;
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
  NiceMock<MockAsyncClientStream> stream;

  EXPECT_CALL(cm_, httpAsyncClientForCluster("egress_cluster"))
      .WillOnce(ReturnRef(cm_.async_client_));
  EXPECT_CALL(cm_.async_client_, start(_, _)).WillOnce(Return(&stream));
  envoy_stream_t stream_id = http_dispatcher_.startStream(observer_);

  Event::PostCb post_cb;
  EXPECT_CALL(event_dispatcher_, post(_)).WillOnce(SaveArg<0>(&post_cb));
  http_dispatcher_.sendHeaders(stream_id, headers, true);

  EXPECT_CALL(event_dispatcher_, isThreadSafe()).Times(3).WillRepeatedly(Return(true));
  EXPECT_CALL(stream, sendHeaders(_, true)).Times(1);
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

} // namespace Http
} // namespace Envoy
