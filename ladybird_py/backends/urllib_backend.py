from __future__ import annotations

import json
import urllib.request
import urllib.error
from typing import Any, Dict

from ..bridge import set_backend, BridgeBackend


class UrllibBackend(BridgeBackend):
    """Bridge backend implemented via Python's urllib for testing and CI.
    This is NOT used in the real browser; it's a stand-in to validate the Python API surface.
    """

    def call(self, name: str, payload: Dict[str, Any]) -> Dict[str, Any]:
        if name == "net.request":
            return self._net_request(payload)
        return {"ok": False, "error": f"unknown call: {name}"}

    async def call_async(self, name: str, payload: Dict[str, Any]) -> Dict[str, Any]:
        # For CI tests we can reuse sync path.
        return self.call(name, payload)

    def _net_request(self, payload: Dict[str, Any]) -> Dict[str, Any]:
        method = payload.get("method", "GET")
        url = payload.get("url")
        headers = payload.get("headers", {}) or {}
        body = payload.get("body")
        json_data = payload.get("json")
        timeout = payload.get("timeout")

        data_bytes = None
        if json_data is not None:
            body = json.dumps(json_data)
            headers.setdefault("Content-Type", "application/json")
        if body is not None:
            data_bytes = body.encode("utf-8") if isinstance(body, str) else body

        req = urllib.request.Request(url=url, data=data_bytes, method=method)
        for k, v in headers.items():
            req.add_header(k, v)

        try:
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                status = resp.getcode()
                resp_headers = {k: v for k, v in resp.getheaders()}
                raw = resp.read()
                try:
                    text = raw.decode("utf-8")
                except Exception:
                    text = raw.decode("latin1", errors="ignore")
                return {"ok": True, "status": status, "headers": resp_headers, "body": text}
        except urllib.error.HTTPError as e:
            raw = e.read() or b""
            try:
                text = raw.decode("utf-8")
            except Exception:
                text = raw.decode("latin1", errors="ignore")
            return {"ok": False, "status": e.code, "error": text}
        except Exception as e:
            return {"ok": False, "error": str(e)}


def set_as_default() -> None:
    set_backend(UrllibBackend())
