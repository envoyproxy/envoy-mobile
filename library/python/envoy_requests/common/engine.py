import atexit
from threading import Event
from typing import Optional

from envoy_engine import Engine
from envoy_engine import EngineBuilder
from envoy_engine import LogLevel


class Engine:
    config_template_override: Optional[str] = None
    _instance: Optional[Engine] = None

    def __init__(self):
        self.handle: Engine

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(Engine, cls).__new__(cls)

            if cls.config_template_override is None:
                engine_builder = EngineBuilder()
            else:
                engine_builder = EngineBuilder(cls.config_template_override)

            engine_running = Event()
            engine = (
                engine_builder.add_log_level(LogLevel.Error).set_on_engine_running(
                    lambda: engine_running.set()
                )
            ).build()
            atexit.register(lambda: engine.terminate())
            engine_running.wait()

            cls._instance.handle = engine
        return cls._instance
