import asyncio
import functools
import threading
from abc import ABC
from abc import abstractmethod
from typing import Any
from typing import cast
from typing import Callable
from typing import Dict
from typing import Generic
from typing import List
from typing import Tuple
from typing import TypeVar

import gevent
from gevent.pool import Group


T = TypeVar("T")
_Func = TypeVar("_Func", bound=Callable[..., Any])


class Executor(ABC):
    # TODO: verify that this preserves type signature
    @abstractmethod
    def wrap(self, fn: _Func) -> _Func:
        pass


class GeventExecutor(Executor):
    def __init__(self):
        self.group = Group()
        self.channel: GeventChannel[
            Tuple[Callable, List[Any], Dict[str, Any]]
        ] = GeventChannel()
        self.spawn_work_greenlet = gevent.spawn(self._spawn_work)

    def __del__(self):
        self.spawn_work_greenlet.kill()

    def wrap(self, fn: _Func) -> _Func:
        @functools.wraps(fn)
        def wrapper(*args, **kwargs):
            self.channel.put((fn, args, kwargs))

        return cast(_Func, wrapper)

    def _spawn_work(self):
        while True:
            (fn, args, kwargs) = self.channel.get()
            self.group.spawn(fn, *args, **kwargs)


class GeventChannel(Generic[T]):
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


class AsyncioExecutor(Executor):
    def __init__(self):
        self.loop = asyncio.get_running_loop()

    def wrap(self, fn: _Func) -> _Func:
        @functools.wraps(fn)
        def wrapper(*args, **kwargs):
            self.loop.call_soon_threadsafe(fn, *args, **kwargs)

        return cast(_Func, wrapper)


class ThreadingExecutor(Executor):
    def __init__(self):
        self.lock = threading.Lock()

    def wrap(self, fn: _Func) -> _Func:
        @functools.wraps(fn)
        def wrapper(*args, **kwargs):
            with self.lock:
                fn(*args, **kwargs)

        return cast(_Func, wrapper)
