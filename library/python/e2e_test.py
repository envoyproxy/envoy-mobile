import threading
from typing import Callable
from typing import Generic
from typing import List
from typing import TypeVar

import gevent
from gevent.event import Event
from gevent.pool import Group

from library.python import envoy_engine


def main():
    on_engine_running_evt = Event()
    on_stream_complete_evt = Event()

    def on_engine_running():
        on_engine_running_evt.set()

    executor = GeventExecutor()
    engine = (
        envoy_engine.EngineBuilder()
        .add_log_level(envoy_engine.LogLevel.Warn)
        .set_on_engine_running(on_engine_running)
        .build(executor)
    )
    on_engine_running_evt.wait()

    def on_headers(headers: envoy_engine.ResponseHeaders, end_stream: bool):
        print("on_headers", headers, end_stream)

    def on_data(data: List[int], end_stream: bool):
        print("on_data", bytes(data[:60]), "...", end_stream)

    def on_trailers(trailers: envoy_engine.ResponseTrailers):
        print("on_trailers", trailers)

    def on_error(error: envoy_engine.EnvoyError):
        print("on_error")
        on_stream_complete_evt.set()

    def on_complete():
        print("on_complete")
        on_stream_complete_evt.set()

    def on_cancel():
        print("on_cancel")
        on_stream_complete_evt.set()

    stream_client = engine.stream_client()
    stream_prototype = (
        stream_client.new_stream_prototype()
        .set_on_headers(on_headers)
        .set_on_data(on_data)
        .set_on_trailers(on_trailers)
        .set_on_error(on_error)
        .set_on_complete(on_complete)
        .set_on_cancel(on_cancel)
    )
    stream = stream_prototype.start(executor)
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
    on_stream_complete_evt.wait()
    # engine.terminate()

T = TypeVar("T")

class ThreadsafeChannel(Generic[T]):
    """
    gevent doesn't play well with multiple real threads. There is no documented canonical way to
    move work across threads, a la asyncio.loop.call_soon_threadsafe. Also, if the main thread is
    waiting on work from another thread, it will raise a LoopExit causing the program to exit.
    This class solves both of those problems by creating a thread-safe way to move data across
    threads that binds with gevent.
    """

    def __init__(self):
        self.hub = gevent.get_hub()
        self.watcher = self.hub.loop.async_()
        self.lock = threading.Lock()
        self.values: List[T] = []

    def put(self, value: T) -> None:
        """
        put enqueues a value and notifies and greenlets waiting on a value that one is available.
        """
        with self.lock:
            self.values.append(value)
            self.watcher.send()

    def get(self) -> T:
        """
        get retrieves an enqueued value. If none exists it waits until notified that such a value is
        available.
        """
        self.lock.acquire()
        while len(self.values) == 0:
            self.lock.release()
            self.hub.wait(self.watcher)
            self.lock.acquire()

        value: T = self.values.pop(0)
        self.lock.release()
        return value


class GeventExecutor(envoy_engine.Executor):
    def __init__(self):
        super().__init__()
        self.group = Group()
        self.channel: ThreadsafeChannel[Callable[[], None]] = ThreadsafeChannel()
        self.spawn_work_greenlet = gevent.spawn(self._spawn_work)

    def __del__(self):
        super().__del__()
        self.spawn_work_greenlet.kill()

    def execute(self, func: Callable[[], None]):
        self.channel.put(func)

    def _spawn_work(self):
        while True:
            self.group.spawn(self.channel.get())


if __name__ == "__main__":
    main()
