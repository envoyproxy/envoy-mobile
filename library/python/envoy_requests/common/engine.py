import atexit
from functools import lru_cache
from threading import Event
from typing import Optional

from envoy_requests.envoy_engine import Engine
from envoy_requests.envoy_engine import EngineBuilder
from envoy_requests.envoy_engine import LogLevel


# TODO: make a better way to change envoy engine config
# include a way to reboot engine on config change(?)
# maybe encapsulate state in global class instance
config_template_override: Optional[str] = None


@lru_cache(maxsize=None)
def make_envoy_engine() -> Engine:
    if config_template_override is None:
        engine_builder = EngineBuilder()
    else:
        engine_builder = EngineBuilder(config_template_override)

    engine_running = Event()
    engine = (
        engine_builder.add_log_level(LogLevel.Error).set_on_engine_running(
            lambda: engine_running.set()
        )
    ).build()
    atexit.register(lambda: engine.terminate())
    engine_running.wait()
    return engine
