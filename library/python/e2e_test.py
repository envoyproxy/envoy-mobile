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


def main():
    move_to_gevent = GeventExecutor()
    engine_running = Event()
    stream_complete = Event()

    @move_to_gevent
    def on_engine_running():
        engine_running.set()

    engine = (
        envoy_engine.EngineBuilder()
        .add_log_level(envoy_engine.LogLevel.Info)
        .set_on_engine_running(on_engine_running)
        .build()
    )

    engine_running.wait()

    @move_to_gevent
    def on_headers(response_headers: envoy_engine.ResponseHeaders, end_stream: bool):
        # TODO: provide iter for header types
        print(response_headers)

    @move_to_gevent
    def on_data(data: bytes, end_stream: bool):
        print(data)

    @move_to_gevent
    def on_complete():
        stream_complete.set()

    @move_to_gevent
    def on_error(error: envoy_engine.EnvoyError):
        stream_complete.set()

    @move_to_gevent
    def on_cancel():
        stream_complete.set()

    stream = (
        engine
        .stream_client()
        .new_stream_prototype()
        .set_on_headers(on_headers)
        .set_on_data(on_data)
        # unused:
        # .set_on_metadata(on_metadata)
        # .set_on_trailers(on_trailers)
        .set_on_complete(on_complete)
        .set_on_error(on_error)
        .set_on_cancel(on_cancel)
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


class GeventExecutor():
    def __init__(self):
        super().__init__()
        self.group = Group()
        self.channel: ThreadsafeChannel[Tuple[Callable, List[Any], Dict[str, Any]]] = ThreadsafeChannel()
        self.spawn_work_greenlet = gevent.spawn(self._spawn_work)

    def __del__(self):
        super().__del__()
        self.spawn_work_greenlet.kill()

    def __call__(self, fn: Callable) -> Callable:
        @functools.wraps(fn)
        def wrapper(*args, **kwargs):
            self.channel.put((fn, args, kwargs))
        return wrapper

    def _spawn_work(self):
        while True:
            (fn, args, kwargs) = self.channel.get()
            self.group.spawn(fn, *args, **kwargs)


T = TypeVar("T")


class ThreadsafeChannel(Generic[T]):
    def __init__(self):
        self.hub = gevent.get_hub()
        self.watcher = self.hub.loop.async_()
        self.lock = threading.Lock()
        self.values: List[T] = []

    def put(self, value: T) -> None:
        with self.lock:
            self.values.append(value)
            self.watcher.send()

    def get(self) -> T:
        self.lock.acquire()
        while len(self.values) == 0:
            self.lock.release()
            self.hub.wait(self.watcher)
            self.lock.acquire()

        value: T = self.values.pop(0)
        self.lock.release()
        return value


if __name__ == "__main__":
    main()
