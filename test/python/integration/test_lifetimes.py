import threading
import unittest

from library.python import envoy_engine
from test.python.integration.common import CONFIG_TEMPLATE


class TestLifetimes(unittest.TestCase):
    def send_request(self, engine: envoy_engine.Engine, stream_complete: threading.Event):
        response = {}
        def _on_headers(response_headers: envoy_engine.ResponseHeaders, end_stream: bool):
            response["status"] = response_headers.http_status()
            response["end_stream"] = end_stream

        stream = (
            engine
            .stream_client()
            .new_stream_prototype()
            .set_on_headers(_on_headers)
            # unused:
            # .set_on_metadata(on_metadata)
            # .set_on_trailers(on_trailers)
            .set_on_complete(lambda: stream_complete.set())
            .set_on_error(lambda _: stream_complete.set())
            .set_on_cancel(lambda: stream_complete.set())
            .start()
        )
        stream.send_headers(
            envoy_engine.RequestHeadersBuilder(
                envoy_engine.RequestMethod.GET,
                "https",
                "example.com",
                "/test",
            )
            .add_upstream_http_protocol(envoy_engine.UpstreamHttpProtocol.HTTP2)
            .build(),
            True,
        )
        return response

    def send_request_end_to_end(self):
        engine_running = threading.Event()
        engine = (
            envoy_engine.EngineBuilder(CONFIG_TEMPLATE)
            .add_log_level(envoy_engine.LogLevel.Error)
            .set_on_engine_running(lambda: engine_running.set())
            .build()
        )
        engine_running.wait()

        stream_complete = threading.Event()
        response = self.send_request(engine, stream_complete)
        stream_complete.wait()
        assert response == {
            "status": 200,
            "end_stream": True,
        }
        engine.terminate()


    def test_lifetimes(self):
        for _ in range(10):
            self.send_request_end_to_end()


if __name__ == "__main__":
    unittest.main(verbosity=2)
