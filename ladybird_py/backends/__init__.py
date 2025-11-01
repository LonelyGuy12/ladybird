# Backends for ladybird_py.bridge
from .urllib_backend import UrllibBackend, set_as_default as set_urllib_backend_default

__all__ = [
    "UrllibBackend",
    "set_urllib_backend_default",
]
