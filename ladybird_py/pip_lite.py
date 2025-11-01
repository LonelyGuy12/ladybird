from __future__ import annotations

import io
import os
import re
import sys
import zipfile
from dataclasses import dataclass
from typing import Iterable, List, Optional, Tuple

from .net import request

PYPI_SIMPLE = "https://pypi.org/simple/"
WHEEL_DIR = os.path.join(os.path.dirname(__file__), "..", "py_wheels")

# Very small resolver: only supports exact pins (package==version) and pure-Python wheels (py3-none-any)


@dataclass
class Requirement:
    name: str
    version: str


def parse_requirements(lines: Iterable[str]) -> List[Requirement]:
    reqs: List[Requirement] = []
    for line in lines:
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        if "==" not in s:
            raise ValueError(f"Only exact pins supported: '{s}'")
        name, version = s.split("==", 1)
        reqs.append(Requirement(name=name.strip().lower(), version=version.strip()))
    return reqs


def _parse_simple_index(name: str, html: str) -> List[Tuple[str, str]]:
    # Return list of (href, filename)
    links: List[Tuple[str, str]] = []
    for m in re.finditer(r"<a\s+href=\"([^\"]+)\"[^>]*>([^<]+)</a>", html, flags=re.IGNORECASE):
        href = m.group(1)
        filename = m.group(2)
        links.append((href, filename))
    return links


def _select_pure_python_wheel(links: List[Tuple[str, str]], name: str, version: str) -> Optional[str]:
    normalized = name.replace("-", "_")
    target_prefix = f"{normalized}-{version}"
    for href, filename in links:
        if not filename.endswith(".whl"):
            continue
        if not filename.startswith(target_prefix):
            continue
        if "py3-none-any" in filename:
            if href.startswith("/"):
                return "https://files.pythonhosted.org" + href
            return href
    return None


def _ensure_wheel_dir() -> str:
    path = os.path.abspath(WHEEL_DIR)
    os.makedirs(path, exist_ok=True)
    return path


def _download(url: str) -> bytes:
    res = request("GET", url)
    if not res.get("ok"):
        raise RuntimeError(f"download failed: {url}: {res.get('error')}")
    body = res.get("body", "")
    return body.encode("utf-8")  # for wheels this will be binary; bridge should deliver bytes in real impl


def _save_and_add_to_path(wheel_bytes: bytes, filename: str) -> None:
    wheel_dir = _ensure_wheel_dir()
    wheel_path = os.path.join(wheel_dir, filename)
    with open(wheel_path, "wb") as f:
        f.write(wheel_bytes)
    if wheel_path not in sys.path:
        sys.path.append(wheel_path)


def install_requirement(req: Requirement) -> None:
    # Fetch simple index page
    index_url = f"{PYPI_SIMPLE}{req.name}/"
    index_res = request("GET", index_url)
    if not index_res.get("ok"):
        raise RuntimeError(f"index fetch failed for {req.name}: {index_res.get('error')}")
    links = _parse_simple_index(req.name, index_res.get("body", ""))
    wheel_url = _select_pure_python_wheel(links, req.name, req.version)
    if not wheel_url:
        raise RuntimeError(f"no pure-Python wheel found for {req.name}=={req.version}")

    wheel_bytes = _download(wheel_url)

    # Validate wheel by attempting to open
    with io.BytesIO(wheel_bytes) as bio:
        with zipfile.ZipFile(bio, "r") as zf:
            _ = zf.namelist()

    filename = wheel_url.split("/")[-1]
    _save_and_add_to_path(wheel_bytes, filename)


def install_requirements_txt(text: str) -> None:
    reqs = parse_requirements(text.splitlines())
    for r in reqs:
        install_requirement(r)
