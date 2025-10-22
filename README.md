# Ladybird with Python Integration
[![build](https://github.com/LonelyGuy12/ladybird/actions/workflows/build.yml/badge.svg)](https://github.com/LonelyGuy12/ladybird/actions/workflows/build.yml)

[Ladybird](https://ladybird.org) is a truly independent web browser, using a novel engine based on web standards.

> [!IMPORTANT]
> Ladybird is in a pre-alpha state, and only suitable for use by developers
>

## 🐍 Python Integration (Experimental)

This fork adds **Python scripting support** to Ladybird, enabling web pages to run Python code alongside JavaScript.

### Goals

- **`<script type="python">`** - Execute Python in web pages with browser API access
- **`requests`-compatible HTTP client** - Familiar Python HTTP library for web requests
- **`aiofiles`-like local storage** - Asynchronous file I/O with browser security
- **`socket.io` connections** - Real-time bidirectional communication
- **Transformers model support** - Run ML models (remote inference, ONNX Runtime, or lightweight runtimes)

### Security Model

- Sandboxed execution environment
- Same-origin policy enforcement
- Restricted module imports (safe modules only)
- No direct filesystem or raw socket access
- Origin-scoped storage

### Current Status

Python integration is under active development. The core infrastructure is in place, with API bindings being updated for the latest Ladybird codebase.

## Features

We aim to build a complete, usable browser for the modern web, now with Python scripting capabilities.

Ladybird uses a multi-process architecture with a main UI process, several WebContent renderer processes,
an ImageDecoder process, and a RequestServer process.

Image decoding and network connections are done out of process to be more robust against malicious content.
Each tab has its own renderer process, which is sandboxed from the rest of the system.

At the moment, many core library support components are inherited from SerenityOS:

- LibWeb: Web rendering engine
- LibJS: JavaScript engine
- LibWasm: WebAssembly implementation
- LibCrypto/LibTLS: Cryptography primitives and Transport Layer Security
- LibHTTP: HTTP/1.1 client
- LibGfx: 2D Graphics Library, Image Decoding and Rendering
- LibUnicode: Unicode and locale support
- LibMedia: Audio and video playback
- LibCore: Event loop, OS abstraction layer
- LibIPC: Inter-process communication

## How do I build and run this?

See [build instructions](Documentation/BuildInstructionsLadybird.md) for information on how to build Ladybird.

Ladybird runs on Linux, macOS, Windows (with WSL2), and many other \*Nixes.

## How do I read the documentation?

Code-related documentation can be found in the [documentation](Documentation/) folder.

## Get in touch and participate!

Join [our Discord server](https://discord.gg/nvfjVJ4Svh) to participate in development discussion.

Please read [Getting started contributing](Documentation/GettingStartedContributing.md) if you plan to contribute to Ladybird for the first time.

Before opening an issue, please see the [issue policy](CONTRIBUTING.md#issue-policy) and the [detailed issue-reporting guidelines](ISSUES.md).

The full contribution guidelines can be found in [`CONTRIBUTING.md`](CONTRIBUTING.md).

## License

Ladybird is licensed under a 2-clause BSD license.
