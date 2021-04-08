from abc import ABC
from abc import abstractmethod
from typing import Any
from typing import Callable
from typing import TypeVar


_Func = TypeVar("_Func", bound=Callable[..., Any])


class Executor(ABC):
    # TODO: verify that this preserves type signature
    @abstractmethod
    def wrap(self, fn: _Func) -> _Func:
        pass
