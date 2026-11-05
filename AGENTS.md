# AGENTS.md

This file provides guidance to AI Agents when working with code in this repository.

## Repository setup

This repo uses [gil](https://github.com/chronoxor/gil) (git-links) instead of git submodules. Dependencies and platform build scripts live in sibling repos listed in `.gitlinks`:

- `modules/Catch2`, `modules/cpp-optparse`, `modules/CppBenchmark`, `modules/CppCommon`, `modules/zlib` — third-party C++ libs vendored as gil links and wired up via `modules/*.cmake`.
- `build/` — platform launcher scripts (`unix.sh`, `vs.bat`, `mingw.bat`, …). **Not present after a plain `git clone`.**
- `cmake/` — shared CMake helpers (`SetCompilerFeatures`, `SetCompilerWarnings`, `SetPlatformFeatures`, `SystemInformation`). Also gil-linked.

Always run `gil update` after cloning or pulling. Without it, `modules/`, `build/`, and `cmake/` are missing and CMake will fail. On Linux, also `apt-get install -y binutils-dev uuid-dev`.

## Build, test, run

All builds go through the platform launcher in `build/`, which invokes CMake and installs binaries to `bin/`:

```shell
cd build && ./unix.sh        # Linux / macOS / Cygwin / MSYS2
cd build && mingw.bat        # Windows MinGW
cd build && vs.bat           # Windows Visual Studio
```

The top-level `CMakeLists.txt` produces:
- `cpplogging` static library (always)
- `cpplogging-tests` (Catch2 single-binary test runner)
- `cpplogging-example-<name>` for every `examples/*.cpp`
- `cpplogging-performance-<name>` for every `performance/*.cpp` (CppBenchmark)
- `binlog`, `hashlog` for every subdir under `tools/`

When this repo is consumed as a sub-module of another project, set `-DCPPLOGGING_MODULE=ON` to skip building tests/examples/benchmarks/tools (see the `if(NOT CPPLOGGING_MODULE)` guard).

### Tests

Run the full Catch2 suite (this is what CTest invokes):
```shell
./bin/cpplogging-tests --durations yes --order lex
ctest --test-dir <build-dir>
```

Run a single test case (Catch2 selects by name or `[tag]`):
```shell
./bin/cpplogging-tests "Format message"
./bin/cpplogging-tests "[CppLogging]"
./bin/cpplogging-tests --list-tests
```

Test sources live flat in `tests/*.cpp`; all share the `[CppLogging]` tag.

## Architecture

CppLogging is a pipeline: `Logger` produces a `Record`, hands it to a `Processor`, which runs it through `Filter`s, hands the survivors to a `Layout` for serialization, and finally fans them out to `Appender`s and any nested sub-processors.

```
Logger → Record → Processor ─┬─ Filters (level / logger name / message regex / switch)
                             ├─ Layout (binary | hash | text | empty | null)
                             ├─ Appenders (console | file | rolling_file | memory | ostream | syslog | null | debug | error)
                             └─ Sub-processors (recursive — same shape)
```

Key types (all under `namespace CppLogging`, headers in `include/logging/`):

- **`Logger`** ([logger.h](include/logging/logger.h)) — front-end. `Debug/Info/Warn/Error/Fatal(fmt::format_string, args...)`. Thread-safety depends on the sink it was bound to.
- **`Record`** ([record.h](include/logging/record.h)) — single logging event. Holds timestamp, thread id, level, logger name, message, an args buffer (for store-format / async), and a `raw` byte vector that layouts populate.
- **`Processor`** ([processor.h](include/logging/processor.h)) — orchestrates one filter/layout/appender chain. Subclasses live in `include/logging/processors/`:
  - `SyncProcessor` — process inline on the calling thread.
  - `AsyncWaitProcessor` / `AsyncWaitFreeProcessor` — hand records to a background thread via a wait-free queue (`async_wait_free_queue.h`). Wait-free variant drops on overflow; wait variant blocks.
  - `BufferedProcessor` — buffer records and flush in batch.
  - `ExclusiveProcessor` — terminates the chain (records aren't passed to siblings).
- **`Layout`** ([layout.h](include/logging/layout.h)) — serializes a `Record` into `record.raw`. `TextLayout` supports a pattern DSL (see Example 7 in `README.md`); `BinaryLayout` writes a compact binary form; `HashLayout` writes only the 32-bit FNV-1a hash of the format string plus arguments — readable only with the `.hashlog` map file.
- **`Appender`** ([appender.h](include/logging/appender.h)) — sink. `RollingFileAppender` ([rolling_file_appender.h](include/logging/appenders/rolling_file_appender.h)) supports time-based and size-based policies and optional Zip archival via the bundled minizip sources under `source/logging/appenders/minizip/`.
- **`Config`** ([config.h](include/logging/config.h)) — static singleton mapping logger name → root processor. `Config::ConfigLogger(name, sink)` registers a logger; `Config::CreateLogger(name)` retrieves one (falls back to default text+console). `Config::Startup()` / `Config::Shutdown()` bracket lifetime — call `Shutdown()` before exit to flush async processors.

`processor.h` pulls in `appenders.h`, `filters.h`, `layouts.h` — those `.h` files just bulk-include all variants in their respective subdirectories. Inline implementations live in `*.inl` next to their headers (e.g. `record.inl`, `logger.inl`, `level.inl`, `async_wait_free_queue.inl`).

## Hash layout & tooling

The hash layout writes 32-bit FNV-1a hashes of format strings instead of the strings themselves — tiny on-disk footprint, but unreadable without a map file. The toolchain:

- **`.hashlog`** — map file (hash → format string) lives at the project root or any ancestor of the binary log.
- **`tools/hashlog`** — converts `*.hash.log` / `*.hash.log.zip` to text. With `--update`, scans a `*.bin.log` and adds discovered strings to `.hashlog`.
- **`tools/binlog`** — converts `*.bin.log` (binary layout) to text.
- **`scripts/hashlog-map/hashlog-map.py`** — parses C++ sources for logging calls and (re)generates `.hashlog`. Installable as `pip3 install hashlog-map`. Run `hashlog-map generate` from the project root.

If you change a format string in code, the old hash is invalidated — regenerate `.hashlog` or run `hashlog --update` over a fresh binary log.

## Platform / compiler notes baked into CMakeLists

- MSVC suppresses `/wd4067 /wd4131 /wd4189 /wd4244 /wd4456` for `cpplogging` (mostly minizip's old C). Non-MSVC adds `-Wno-shadow -Wno-unused-variable`. If you're touching warnings, expect to update `CMakeLists.txt:49-58`.
- Cygwin and MinGW define `USE_FILE32API=1` (minizip 32-bit file API).
- Windows builds compile `source/logging/appenders/minizip/iowin32.c`; non-Windows builds explicitly remove it from the glob.
