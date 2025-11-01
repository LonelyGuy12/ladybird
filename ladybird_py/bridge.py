# Minimal Python-native bridge API (no JS dependency)
# Provides a process-internal call gateway that Python shims use to reach the browser engine.

from __future__ import annotations

import asyncio
from typing import Any, Dict, Optional, Protocol


class BridgeBackend(Protocol):
    def call(self, name: str, payload: Dict[str, Any]) -> Dict[str, Any]:
        ...

    async def call_async(self, name: str, payload: Dict[str, Any]) -> Dict[str, Any]:
        ...


class _DefaultBackend:
    def call(self, name: str, payload: Dict[str, Any]) -> Dict[str, Any]:
        raise RuntimeError("No bridge backend configured")

    async def call_async(self, name: str, payload: Dict[str, Any]) -> Dict[str, Any]:
        # Fallback to sync if no async provided
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self.call, name, payload)


_backend: BridgeBackend = _DefaultBackend()


def set_backend(backend: BridgeBackend) -> None:
    global _backend
    _backend = backend


def get_backend() -> BridgeBackend:
    return _backend


def call(name: str, payload: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
    return _backend.call(name, payload or {})


async def call_async(name: str, payload: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
    return await _backend.call_async(name, payload or {})
