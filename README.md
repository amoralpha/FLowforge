# FlowForge

A production-quality **workflow orchestration and execution engine** written in C++23.

FlowForge takes DAG-based workflow definitions, resolves task dependencies, schedules tasks, executes them through workers, handles failures/retries, and persists execution state.

## Status

**v0.1 — Architecture scaffolding complete.** No implementation code yet. See the architecture document below for the full design.

## Requirements

- C++23 compatible compiler (GCC 13+, Clang 17+, MSVC 19.35+)
- CMake 3.28+
- Ninja (recommended generator)

## Building

```bash
# Configure (development)
cmake --preset dev -G Ninja
cmake --build build/dev

# Configure (release)
cmake --preset release -G Ninja
cmake --build build/release

# Run tests
ctest --test-dir build/dev

# Run benchmarks
./build/release/benchmarks/flowforge_benchmarks
```

## Project Structure

```
FlowForge/
├── CMakeLists.txt              # Top-level build definition
├── CMakePresets.json           # Build presets (dev, release, relwithdebinfo)
├── LICENSE                     # MIT License
├── README.md                   # This file
├── .gitignore                  # Git ignore rules
├── .clang-format               # C++ code formatting (Google-based, 100 col)
├── .clang-tidy                 # Static analysis checks
├── .editorconfig               # Editor consistency settings
│
├── include/flowforge/          # PUBLIC API HEADERS (installed with the library)
│   ├── core/                   #   Fundamental types, errors, concepts
│   ├── dag/                    #   DAG representation
│   ├── task/                   #   Task abstraction
│   ├── scheduler/              #   Scheduling logic
│   ├── executor/               #   Task execution
│   ├── worker/                 #   Worker pool
│   ├── runtime/                #   Main runtime orchestrator
│   ├── storage/                #   Persistence (PostgreSQL, future backends)
│   ├── cache/                  #   Content-based caching (BLAKE3)
│   ├── docker/                 #   Docker-based task isolation
│   ├── distributed/            #   Distributed workers (future)
│   ├── logging/                #   Structured logging
│   ├── metrics/                #   Observability / metrics
│   └── cli/                    #   CLI application
│
├── src/
│   ├── flowforge/              # LIBRARY IMPLEMENTATION (mirrors include/)
│   │   ├── core/
│   │   ├── dag/
│   │   ├── task/
│   │   ├── scheduler/
│   │   ├── executor/
│   │   ├── worker/
│   │   ├── runtime/
│   │   ├── storage/
│   │   ├── cache/
│   │   ├── docker/
│   │   ├── distributed/
│   │   ├── logging/
│   │   ├── metrics/
│   │   └── cli/
│   └── tools/                  # EXECUTABLES
│       └── flowforge.cpp       #   CLI entry point
│
├── tests/                      # FIRST-CLASS TESTING
│   ├── CMakeLists.txt
│   ├── test_main.cpp
│   ├── unit/                   #   Unit tests (per module)
│   ├── integration/            #   Cross-module integration tests
│   └── fixtures/               #   Shared test data / helpers
│
├── benchmarks/                 # FIRST-CLASS BENCHMARKING
│   ├── CMakeLists.txt
│   ├── bench_main.cpp
│   ├── bench_dag.cpp
│   ├── bench_scheduler.cpp
│   ├── bench_executor.cpp
│   └── bench_worker_pool.cpp
│
├── examples/                   # Runnable examples
│   ├── simple_workflow.cpp
│   └── parallel_workflow.cpp
│
├── docs/                       # DOCUMENTATION
│   ├── index.md
│   ├── getting-started.md
│   ├── architecture.md         #   This document
│   ├── api.md                  #   Generated API reference
│   ├── design/                 #   Design decision records (DDRs)
│   │   ├── dag-design.md
│   │   ├── scheduler-design.md
│   │   └── persistence-design.md
│   ├── examples/               #   Documentation code samples
│   └── api/                    #   Doxygen-generated HTML
│
├── scripts/                    # AUTOMATION & CI
│   ├── ci/
│   │   ├── github/             #   GitHub Actions workflows
│   │   └── docker/             #   Container build scripts
│   ├── setup/                  #   Developer environment setup
│   └── dev/                    #   Developer convenience scripts
│
├── cmake/                      # CMAKE INFRASTRUCTURE
│   ├── CompilerSettings.cmake  #   C++23 standard, warnings, sanitizers
│   ├── Dependencies.cmake      #   External dependency discovery
│   ├── Testing.cmake           #   Test framework setup (Catch2)
│   ├── Benchmarking.cmake      #   Benchmark setup (Google Benchmark)
│   └── FlowForgeConfig.cmake.in #   Package config for find_package
│
└── third_party/                # VENDORED DEPENDENCIES (if any)
    └── README.md               #   Vendoring policy
```

## Module Dependency Graph

```
                    ┌─────────────┐
                    │     CLI     │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │   Runtime   │
                    └──────┬──────┘
                           │
           ┌───────────────┼───────────────┐
           │               │               │
    ┌──────▼──────┐ ┌──────▼──────┐ ┌──────▼──────┐
    │  Scheduler  │ │  Executor   │ │   Worker    │
    └──────┬──────┘ └──────┬──────┘ └──────┬──────┘
           │               │               │
           └───────┬───────┘               │
                   │                       │
            ┌──────▼──────┐                │
            │     DAG     │                │
            └──────┬──────┘                │
                   │                       │
            ┌──────▼──────┐                │
            │    Task     │◄───────────────┘
            └──────┬──────┘
                   │
            ┌──────▼──────┐
            │    Core     │
            └─────────────┘

Future modules (do NOT create circular dependencies):
  Storage ──────► Task, DAG, Core
  Cache ─────────► Task, Core
  Docker ────────► Executor, Task, Core
  Distributed ───► Worker, Executor, Task, Core
  Logging ───────► Core (cross-cutting, injected everywhere)
  Metrics ───────► Core (cross-cutting, injected everywhere)
```

**Rule:** Modules may only depend on modules "below" them. No upward or circular dependencies. This is enforced by CMake target link relationships.

## v0.1 Modules (Implement Now)

These modules have full implementation in v0.1:

| Module | Purpose |
|--------|---------|
| **core** | Fundamental types (`TaskId`, `WorkflowId`), `std::expected` wrappers, error codes, C++23 concepts |
| **task** | `Task` class, `TaskConfig` (timeout, retries), `TaskResult`, `TaskStatus` enum |
| **dag** | `DAG`, `Node`, `Edge`, topological sort, cycle detection |
| **scheduler** | `Scheduler` interface + `TopologicalScheduler` implementation |
| **executor** | `IExecutor` interface, `SyncExecutor` (synchronous, in-process) |
| **worker** | `WorkerPool`, `Worker`, `TaskQueue` (bounded MPMC queue) |
| **runtime** | `Runtime` orchestrator: loads DAG, schedules, executes via workers |
| **cli** | `CLI` application using CLI11: `run`, `status` subcommands |
| **logging** | Minimal `ILogger` interface + `ConsoleLogger` for v0.1 observability |

## Future Modules (Stub Only — Empty or Placeholder)

These directories exist so the architecture is complete, but implementations are deferred:

| Module | Planned For | Why Deferred |
|--------|-------------|--------------|
| **storage** | v0.3+ | PostgreSQL persistence requires schema design and libpqxx integration |
| **cache** | v0.4+ | Content-based caching with BLAKE3 requires stable task output contracts |
| **docker** | v0.5+ | Docker isolation requires Docker SDK / subprocess orchestration |
| **distributed** | v0.6+ | Distributed workers require network protocol and message queue |
| **metrics** | v0.2+ | Prometheus/metrics exposition can be added once runtime is stable |

For v0.1, these directories contain only a placeholder `CMakeLists.txt` and an empty umbrella header if needed. They are not added to the build.

## Directory Responsibilities

### `include/flowforge/` — Public API
- **What belongs here:** Every header that external consumers of FlowForge may include. This is the stable interface boundary.
- **What must NOT go here:** Internal implementation headers, pimpl details, private helpers, `.cpp` files.
- **Architectural role:** This is the **only** directory installed by `make install`. Changes here are API changes and follow semver.

### `src/flowforge/` — Implementation
- **What belongs here:** All `.cpp` files, internal headers (pimpl, helpers), module-private details.
- **What must NOT go here:** Public headers (those belong in `include/`).
- **Architectural role:** This is **not installed**. It contains the compiled translation units. The directory structure mirrors `include/` so implementation is easy to locate for any header.

### `src/tools/` — Executables
- **What belongs here:** `main()` functions and thin CLI wiring.
- **What must NOT go here:** Library code, reusable components.
- **Architectural role:** Keeps the library (`src/flowforge/`) separate from delivered binaries. Multiple tools (CLI, future daemons) can coexist here.

### `tests/` — Testing
- **What belongs here:** All test code.
  - `tests/unit/<module>/` — Unit tests for the corresponding `src/flowforge/<module>/`.
  - `tests/integration/` — Tests that span multiple modules (e.g., Runtime + DAG + Executor).
  - `tests/fixtures/` — Shared test data, sample workflows, helper utilities.
- **What must NOT go here:** Production code, benchmarks (those belong in `benchmarks/`).
- **Architectural role:** First-class testing ensures every public API has a corresponding test. Mirroring the `src/` structure makes it obvious which tests cover which code.

### `benchmarks/` — Performance Benchmarks
- **What belongs here:** Google Benchmark microbenchmarks for performance-critical modules.
- **What must NOT go here:** Unit tests, production code.
- **Architectural role:** Benchmarks are opt-in (disabled in dev by default, enabled in dedicated CI jobs). They catch performance regressions.

### `examples/` — Runnable Examples
- **What belongs here:** Standalone C++ programs that demonstrate FlowForge usage.
- **What must NOT go here:** Library internals, test code.
- **Architectural role:** Examples serve as both documentation and integration smoke tests.

### `docs/` — Documentation
- **What belongs here:**
  - `index.md`, `getting-started.md` — User-facing docs.
  - `architecture.md` — This document.
  - `design/` — Design Decision Records (DDRs) for major architectural choices.
  - `api/` — Doxygen-generated API reference (gitignored, generated in CI).
  - `examples/` — Documentation code samples (not runnable, used in markdown).
- **What must NOT go here:** Implementation code, internal notes.

### `scripts/` — Automation
- **What belongs here:**
  - `ci/` — GitHub Actions workflows, Docker build scripts.
  - `setup/` — Developer environment bootstrap.
  - `dev/` — Convenience scripts (`format.sh`, `lint.sh`, `run_tests.sh`).
- **What must NOT go here:** C++ source code.

### `cmake/` — CMake Infrastructure
- **What belongs here:** Reusable CMake logic: compiler flags, dependency discovery, test/benchmark setup, package config templates.
- **What must NOT go here:** Target definitions, source file lists (those belong in the respective `CMakeLists.txt`).

### `third_party/` — Vendored Dependencies
- **What belongs here:** Only if a dependency cannot be obtained via system package manager or CMake FetchContent. Each vendored library gets its own subdirectory.
- **What must NOT go here:** Random utility scripts.
- **Policy:** See `third_party/README.md`. Vendoring is discouraged; prefer CMake FetchContent with pinned revisions.

## Architectural Rules

1. **Dependency Direction:** Modules may only depend on modules "below" them in the dependency graph. No circular or upward dependencies. Enforced by CMake `target_link_libraries`.

2. **Public API Only in `include/`:** All external-facing headers live in `include/flowforge/`. Internal headers live in `src/flowforge/` only. Never include a `src/` header from outside the library.

3. **Include What You Use:** Every header includes its own dependencies. Do not rely on transitive includes from other FlowForge headers. This prevents fragile include-order bugs.

4. **No Exceptions Across Module Boundaries:** Use `std::expected<T, E>` (C++23) or `flowforge::Expected<T, E>` for recoverable errors. Exceptions are documented as undefined behavior unless explicitly marked `noexcept(false)`.

5. **No Global State:** Avoid singletons. Pass dependencies (logger, storage backend, cache backend) via constructor injection.

6. **One Public Header Per Module:** Each module has a primary header (e.g., `include/flowforge/runtime/runtime.hpp`) that includes sub-headers. External code includes only the primary header.

7. **CMake Targets Match Modules:** Each module directory builds into a dedicated CMake target (`flowforge::core`, `flowforge::dag`, etc.). This makes dependency boundaries explicit and compile times faster.

8. **No Pimpl in v0.1:** Concrete classes with stable layout. Pimpl indirection can be introduced later if binary compatibility becomes a requirement.

9. **Test Coverage Rule:** Every public API function and class must have at least one corresponding unit test. Integration tests cover cross-module workflows.

10. **Benchmarking Rule:** Any module with a hot path (DAG traversal, scheduling, worker queue) must have a corresponding benchmark. Benchmarks live in `benchmarks/` and are run in CI on every PR.

11. **Documentation Rule:** Every public class and function has a Doxygen comment. Design decisions are recorded as DDRs in `docs/design/`.

12. **No External Dependency Leakage:** If FlowForge uses a third-party library (e.g., nlohmann/json, BLAKE3), the types are wrapped in FlowForge-specific types. External consumers never see the third-party headers unless explicitly intended.

13. **Incremental Enablement:** Future modules exist as empty directories with a stub `CMakeLists.txt` so the architecture is complete, but they are not added to the build in v0.1. This prevents "works on my machine" surprises when the module is finally implemented.

14. **Concurrency Safety:** Every public class documents its thread-safety guarantees: `@threadsafe`, `@not_threadsafe`, or `@threadsafe_after_init`.

## Internal Module Design

### Core
- `concepts.hpp` — C++23 concepts: `TaskConcept`, `ExecutorConcept`, `SchedulerConcept`.
- `error.hpp` — `Error` type, error categories, `make_error()`.
- `expected.hpp` — Thin wrapper around `std::expected` with FlowForge-specific utilities.
- `id.hpp` — Strongly-typed IDs: `TaskId`, `WorkflowId`, `ExecutionId` (strong typedefs preventing mix-ups).
- `status.hpp` — Common status enums: `TaskStatus`, `ExecutionPhase`.

### Task
- `task.hpp` — `Task` class: name, command/function, config, metadata.
- `task_config.hpp` — `TaskConfig`: timeout, retries, environment variables, resource limits.
- `task_result.hpp` — `TaskResult`: exit code, stdout, stderr, duration, artifact paths.

### DAG
- `dag.hpp` — `DAG` class: owns nodes and edges, provides topological order.
- `node.hpp` — `Node`: wraps a `TaskId`, adds node-level metadata.
- `edge.hpp` — `Edge`: source `TaskId`, target `TaskId`.
- `topological_sort.hpp` — Kahn's algorithm / DFS-based sort.
- `cycle_detection.hpp` — DFS-based cycle detection with path reporting.

### Scheduler
- `scheduler.hpp` — `IScheduler` interface: `schedule()` returns next batch of runnable tasks.
- `topological_scheduler.hpp` — Default implementation: priority-aware topological scheduling.

### Executor
- `executor.hpp` — `IExecutor` interface: `execute(task, context)`.
- `execution_context.hpp` — `ExecutionContext`: runtime state, working directory, env.
- `sync_executor.hpp` — `SyncExecutor`: runs task synchronously in-process (v0.1).

### Worker
- `worker.hpp` — `Worker`: owns a thread, pulls tasks from queue, executes via `IExecutor`.
- `worker_pool.hpp` — `WorkerPool`: manages N workers, bounded task queue, shutdown semantics.
- `task_queue.hpp` — Bounded MPMC queue (lock-free or mutex-based).

### Runtime
- `runtime.hpp` — `Runtime`: main user-facing orchestrator. Loads DAG, coordinates scheduler, executor, workers.
- `workflow_runner.hpp` — `WorkflowRunner`: executes a single workflow instance, emits events.
- `execution_state.hpp` — `ExecutionState`: tracks which tasks are pending/running/completed/failed.
- `events.hpp` — Event system: `TaskStartedEvent`, `TaskCompletedEvent`, `TaskFailedEvent`.

### Storage (future)
- `storage_backend.hpp` — `IStorageBackend` interface.
- `postgresql_storage.hpp` — PostgreSQL implementation using libpqxx.
- `repositories/` — Typed repositories: `WorkflowRepository`, `TaskRepository`, `ExecutionRepository`.

### Cache (future)
- `cache_backend.hpp` — `ICacheBackend` interface.
- `content_hash_cache.hpp` — BLAKE3-based content-addressable cache for task outputs.

### Docker (future)
- `docker_executor.hpp` — `DockerExecutor`: executes tasks inside Docker containers.

### Distributed (future)
- `distributed_worker_pool.hpp` — `DistributedWorkerPool`: workers connect over network.
- `task_queue.hpp` — Network-transparent task queue (future protocol).

### Logging
- `logger.hpp` — `ILogger` interface: `log(Level, message)`.
- `console_logger.hpp` — `ConsoleLogger`: human-readable console output (v0.1).

### Metrics (future)
- `metrics_collector.hpp` — `IMetricsCollector` interface.
- `prometheus_exporter.hpp` — Prometheus text format exposition.

### CLI
- `cli_app.hpp` — `CLI` class: parses args, dispatches to commands.
- `commands/` — `RunCommand`, `StatusCommand`, etc.

## Versioning & Compatibility

- **v0.x:** Public API may change between minor versions. No ABI stability guarantee.
- **v1.0+:** Public API is stable. ABI stability is guaranteed within the same major version.

## Contributing

See `CONTRIBUTING.md` (to be added). Key points:
- Format with `clang-format` before committing.
- All tests and benchmarks must pass in CI.
- Design changes require a DDR in `docs/design/`.


## Benchmarking

### Environment

- Build configuration: Release
- Compiler: MSVC 19.50.35725
- Benchmark framework: Google Benchmark v1.9.1
- Logical CPUs reported: 16
- Repetitions: 5

### DAG Performance Baseline

| Benchmark | Tasks | Mean Time | CV |
|---|---:|---:|---:|
| DAG Insert | 100 | 44.27 µs | 6.82% |
| DAG Insert | 1,000 | 551.30 µs | 23.10% |
| DAG Insert | 10,000 | 6.52 ms | 3.36% |
| DAG Validate — Independent | 100 | 106.42 µs | 4.92% |
| DAG Validate — Independent | 1,000 | 1.18 ms | 6.71% |
| DAG Validate — Independent | 10,000 | 18.20 ms | 16.07% |
| DAG Validate — Chain | 100 | 103.19 µs | 5.38% |
| DAG Validate — Chain | 1,000 | 1.33 ms | 4.13% |
| DAG Validate — Chain | 10,000 | 22.13 ms | 11.82% |

### Methodology and Limitations

- Results are preliminary and were collected using five repetitions.
- DAG insertion measurements include task-name construction and container operations.
- DAG validation measurements cover missing-dependency checking, cycle detection, and topological sorting.
- Independent graphs contain no dependency edges; chain graphs contain N − 1 edges.
- Results may vary with system load and hardware conditions.
- No performance optimization has been applied against this baseline yet.