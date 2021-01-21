import functools
import threading
from typing import Any
from typing import Callable
from typing import Dict
from typing import Generic
from typing import List
from typing import TypeVar

import gevent
from gevent.event import Event
from gevent.pool import Group

from library.python import envoy_engine
from library.python.gevent_util import GeventEngineBuilder
from library.python.gevent_util import GeventExecutor


def main():
    engine_running = Event()
    engine = (
        GeventEngineBuilder()
        .add_log_level(envoy_engine.LogLevel.Error)
        .set_on_engine_running(lambda: engine_running.set())
        .build()
    )
    engine_running.wait()

    stream_complete = Event()
    stream = (
        engine
        .stream_client()
        .new_stream_prototype()
        .set_on_headers(lambda response_headers, end_stream: print(response_headers, end_stream))
        .set_on_data(lambda data, end_stream: print(data, end_stream))
        # unused:
        # .set_on_metadata(on_metadata)
        # .set_on_trailers(on_trailers)
        .set_on_complete(lambda: stream_complete.set())
        .set_on_error(lambda error: stream_complete.set())
        .set_on_cancel(lambda cancel: stream_complete.set())
        .start()
    )
    stream.send_headers(
        envoy_engine.RequestHeadersBuilder(
            envoy_engine.RequestMethod.GET,
            "https",
            "www.google.com",
            "/",
        )
        .build(),
        True,
    )
    stream_complete.wait()

if __name__ == "__main__":
    main()
