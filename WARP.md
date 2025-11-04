# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

Project overview
- Ladybird is a multi-process browser: Browser UI orchestrates sandboxed helpers per tab.
  - WebContent: HTML/CSS engine (LibWeb) and JS (LibJS); paints to shared bitmaps.
  - RequestServer: out-of-process networking (HTTP/TLS, DNS via system LookupServer).
  - ImageDecoder: out-of-process image decoding.
- Key libraries: LibWeb (engine), LibJS (JS VM), LibWasm, LibGfx, LibHTTP, LibTLS, LibCore, etc.
- Python integration (experimental in this fork):
  - HTMLScriptElement recognizes type="python" and executes via PythonScript through PythonEngine.
  - Cross-language bridge exposes DOM APIs to Python (Bindings::PythonDOMAPI) and JS↔Python interop (PythonJSBridge).
  - PythonSecurityModel restricts builtins/imports and applies resource limits; adjust when enabling features.

Common commands
- Build and run
  - ./Meta/ladybird.py run
  - BUILD_PRESET=Debug ./Meta/ladybird.py run
  - macOS with Homebrew clang: CC=$(brew --prefix llvm)/bin/clang CXX=$(brew --prefix llvm)/bin/clang++ ./Meta/ladybird.py run
  - Run app directly after build (Linux): ./Build/release/bin/Ladybird
  - Run app bundle (macOS): open -W --stdout $(tty) --stderr $(tty) ./Build/release/bin/Ladybird.app --args https://ladybird.dev
- CMake presets (alternative to ladybird.py)
  - cmake --preset Release && cmake --build --preset Release
  - Custom dir: cmake --preset default -B MyBuildDir && cmake --build --preset default MyBuildDir
- Enable optional features
  - Sanitizers: cmake --preset Sanitizer && cmake --build --preset Sanitizer
  - Qt UI (when not default): cmake --preset default -DENABLE_QT=ON
  - Python integration: add -DENABLE_PYTHON=ON if available on your platform (ensure Python dev libs installed)
- Tests
  - All tests: ./Meta/ladybird.py test
  - Only LibWeb tests: ./Meta/ladybird.py test LibWeb
  - Run test-web directly: ./Meta/ladybird.py run test-web
  - ctest with preset: cmake --preset Release && cmake --build --preset Release && ctest --preset Release
  - Show failing test output: CTEST_OUTPUT_ON_FAILURE=1 ninja test (from Build/release)
  - Run a single test file (Text/Layout): ./Meta/ladybird.py run test-web -f Text/input/your-test.html
  - Web Platform Tests: ./Meta/WPT.sh run --log results.log [subset] and ./Meta/WPT.sh compare --log new.log prev.log [subset]
- Lint/format and CI parity
  - Quick style checks: ninja -C Build/release check-style
  - Shell scripts: ninja -C Build/release lint-shell-scripts
  - CI lint script (macOS tooling expected): Meta/lint-ci.sh
  - Python lint (ruff configured): ruff check .
- Android
  - UI/Android/gradlew tasks or follow Documentation/EditorConfiguration/AndroidStudioConfiguration.md

Notes and platform specifics
- Windows builds are supported via WSL2; native Windows is experimental (see Documentation/BuildInstructionsLadybird.md).
- Resource files from Base/res are copied into build by CMake; ensure they’re present when packaging.
- Experimental GN build exists; see Meta/gn/README.md for gn gen out && ninja -C out.

High-level code structure landmarks (big picture)
- UI frontends: UI/AppKit (macOS), UI/Qt (cross-platform), UI/Android.
- Services processes: Services/WebContent, Services/RequestServer, Services/ImageDecoder, Services/WebWorker, Services/WebDriver.
- Engine and plumbing live in Libraries/*; LibWeb is the central engine with HTML/DOM/CSS/Script loading/execution.
- Python integration touchpoints:
  - Libraries/LibWeb/HTML/HTMLScriptElement.*: detects and executes Python scripts.
  - Libraries/LibWeb/HTML/Scripting/PythonScript.*: compilation and execution via CPython API.
  - Libraries/LibWeb/HTML/Scripting/PythonEngine.*: interpreter lifecycle.
  - Libraries/LibWeb/HTML/Scripting/PythonSecurityModel.*: sandbox and allowed builtins/modules.
  - Libraries/LibWeb/Bindings/PythonDOMBindings.* and PythonJSBridge.*: DOM exposure and interop.

Troubleshooting and dev tips specific to this repo
- Tests sometimes need LADYBIRD_SOURCE_DIR exported to the repo root before running.
- If third-party deps fail (vcpkg), inspect Build/release/vcpkg-manifest-install.log.
- On macOS, launching via open -W … ensures stdout/stderr reach the terminal for debugging multiprocess logs.
- For debugging layout/rendering: attach to WebContent processes in your IDE.

Key docs to skim first
- Documentation/BuildInstructionsLadybird.md (setup, presets, running)
- Documentation/Testing.md (test types, running single tests, rebaselining)
- Documentation/ProcessArchitecture.md (multi-process design)
- docs/python_integration/* (architecture, security, build integration, testing strategy)
