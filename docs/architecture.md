# FlowForge Architecture

This document describes the complete repository structure, module boundaries, dependency graph, and architectural rules for FlowForge.

For quick navigation, see [README.md](../README.md).

## Table of Contents

1. [Complete Folder Tree](#complete-folder-tree)
2. [Directory Responsibilities](#directory-responsibilities)
3. [Module Dependency Graph](#module-dependency-graph)
4. [v0.1 Modules](#v01-modules)
5. [Future Modules](#future-modules)
6. [Architectural Rules](#architectural-rules)
7. [Internal Module Design](#internal-module-design)

---

## Complete Folder Tree

```
FlowForge/
├── CMakeLists.txt              # Top-level CMake build
├── CMakePresets.json           # Build presets (dev, release, relwithdebinfo)
├── LICENSE                     # MIT License
├── README.md                   # Project overview and quick start
├── .gitignore                  # Git ignore rules
├── .clang-format               # C++ formatting (Google style, 100 cols)
├── .clang-tidy                 # Static analysis configuration
├── .editorconfig               # Editor consistency settings
│
├── include/flowforge/          # PUBLIC API HEADERS (installed with library)
│   ├── flowforge.hpp           #   Umbrella header
│   ├── core/                   #   Fundamental types, errors, concepts
│   ├── dag/                    #   DAG representation
│   ├── task/                   #   Task abstraction
│   ├── scheduler/              #   Scheduling logic
│   ├── executor/               #   Task execution
│   ├── worker/                 #   Worker pool
│   ├── runtime/                #   Main runtime orchestrator
│   ├── storage/                #   Persistence (future)
│   ├── cache/                  #   Content-based caching (future)
│   ├── docker/                 #   Docker isolation (future)
│   ├── distributed/            #   Distributed workers (future)
│   ├── logging/                #   Structured logging
│   ├── metrics/                #   Observability / metrics (future)
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
│   ├── unit/                   #   Unit tests per module
│   ├── integration/            #   Cross-module integration tests
│   └── fixtures/               #   Shared test data / helpers
│
├── benchmarks/                 # FIRST-CLASS BENCHMARKING
│   ├── CMakeLists.txt
│   ├── bench_main.cpp
│   ├── bench_dag.cpp
│   ├── bench_scheduler.cpp
│   ├── bench_executor.cpp
│   ├── bench_worker_pool.cpp
│   └── bench_runtime.cpp
│
├── examples/                   # RUNNABLE EXAMPLES
│   ├── simple_workflow.cpp
│   ├── parallel_workflow.cpp
│   └── CMakeLists.txt
│
├── docs/                       # DOCUMENTATION
│   ├── index.md
│   ├── getting-started.md
│   ├── architecture.md         #   This document
│   ├── api.md                  #   Generated API reference (Doxygen)
│   ├── design/                 #   Design Decision Records (DDRs)
│   │   ├── dag-design.md
│   │   ├── scheduler-design.md
│   │   └── persistence-design.md
│   ├── examples/               #   Documentation code samples
│   └── api/                    #   Doxygen-generated HTML (gitignored)
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
└── third_party/                # VENDORED DEPENDENCIES
    └── README.md               #   Vendoring policy
```

---

## Directory Responsibilities

### `include/flowforge/` — Public API

- **Why it exists:** This is the **stable interface boundary**. Everything here is installed by `make install` and is visible to consumers of FlowForge.
- **What belongs here:** Public headers, C++23 concepts, abstract interfaces, and stable type definitions.
- **What must NOT go here:** Internal implementation headers, pimpl details, private helpers, `.cpp` files.
- **Future modules:** All 14 modules (Core, DAG, Task, Scheduler, Executor, Worker, Runtime, Storage, Cache, Docker, Distributed, Logging, Metrics, CLI) have their public headers here.
- **Architectural coupling prevention:** By making this the ONLY installed directory, we control exactly what external code can see. Internal refactoring never breaks consumers as long as this directory's contract is honored.

### `src/flowforge/` — Implementation

- **Why it exists:** Contains compiled translation units and internal headers. This directory is **not installed**.
- **What belongs here:** All `.cpp` files, internal headers (pimpl, helpers), module-private implementation details.
- **What must NOT go here:** Public headers (those belong in `include/`). No executable `main()` functions.
- **Future modules:** Mirrors `include/flowforge/` exactly, making it trivial to locate the `.cpp` for any header.
- **Architectural coupling prevention:** Internal details are hidden from consumers. We can refactor implementation freely without affecting the public API.

### `src/tools/` — Executables

- **Why it exists:** Separates delivered binaries from the library code.
- **What belongs here:** `main()` functions, thin CLI wiring, and tool-specific logic.
- **What must NOT go here:** Reusable library code. Everything here should depend on `src/flowforge/`, never the reverse.
- **Future modules:** `flowforge` (CLI), potential future tools (daemon, migration tool, etc.).
- **Architectural coupling prevention:** Keeps the library (`src/flowforge/`) independent of any specific delivery mechanism.

### `tests/` — Testing

- **Why it exists:** First-class testing ensures correctness and prevents regressions.
- **What belongs here:**
  - `tests/unit/<module>/` — Unit tests for the corresponding `src/flowforge/<module>/`.
  - `tests/integration/` — Cross-module integration tests (e.g., Runtime + DAG + Executor).
  - `tests/fixtures/` — Shared test data, sample workflows, helper utilities.
- **What must NOT go here:** Production code, benchmarks (those belong in `benchmarks/`).
- **Future modules:** Every future module gets a corresponding `tests/unit/<module>/` directory when implemented.
- **Architectural coupling prevention:** Mirroring the `src/` structure makes it obvious which tests cover which code, preventing orphaned or forgotten tests.

### `benchmarks/` — Performance Benchmarks

- **Why it exists:** Performance is a first-class concern for a systems engine. Benchmarks catch regressions before they reach production.
- **What belongs here:** Google Benchmark microbenchmarks for hot paths (DAG traversal, scheduling, worker queue, runtime startup).
- **What must NOT go here:** Unit tests, production code.
- **Future modules:** Every performance-critical module gets a benchmark file when implemented.
- **Architectural coupling prevention:** Benchmarks are opt-in and run separately from unit tests, keeping the main test suite fast.

### `examples/` — Runnable Examples

- **Why it exists:** Examples serve as both documentation and integration smoke tests.
- **What belongs here:** Standalone C++ programs demonstrating FlowForge usage (simple workflow, parallel workflow, retry handling).
- **What must NOT go here:** Library internals, test code.
- **Future modules:** Examples grow as new modules are exposed.
- **Architectural coupling prevention:** Examples consume only the public API, serving as a canary for API breakage.

### `docs/` — Documentation

- **Why it exists:** Documentation is a first-class deliverable, not an afterthought.
- **What belongs here:**
  - User-facing docs (`index.md`, `getting-started.md`).
  - `architecture.md` — This document.
  - `design/` — Design Decision Records (DDRs) for major architectural choices.
  - `api/` — Doxygen-generated API reference (gitignored, built in CI).
  - `examples/` — Documentation code samples (not runnable, used in markdown).
- **What must NOT go here:** Implementation code, internal notes.
- **Future modules:** As new modules are added, their DDRs and API docs go here.
- **Architectural coupling prevention:** Documentation is decoupled from code. DDRs explain *why* decisions were made, preventing repeated debates.

### `scripts/` — Automation & CI

- **Why it exists:** Reproducible builds, CI pipelines, and developer convenience.
- **What belongs here:**
  - `ci/` — GitHub Actions workflows, Docker build scripts.
  - `setup/` — Developer environment bootstrap scripts.
  - `dev/` — Convenience scripts (`format.sh`, `lint.sh`, `run_tests.sh`).
- **What must NOT go here:** C++ source code, build system logic (that belongs in `cmake/`).
- **Future modules:** CI configs expand as new modules are added.
- **Architectural coupling prevention:** Scripts are external to the library, so build/test logic doesn't leak into production code.

### `cmake/` — CMake Infrastructure

- **Why it exists:** Centralizes build logic, keeping `CMakeLists.txt` files clean and consistent.
- **What belongs here:**
  - `CompilerSettings.cmake` — C++23 standard, warnings, sanitizers, LTO.
  - `Dependencies.cmake` — External dependency discovery (find_package + FetchContent).
  - `Testing.cmake` — Test framework setup (Catch2).
  - `Benchmarking.cmake` — Benchmark setup (Google Benchmark).
  - `FlowForgeConfig.cmake.in` — Package config template for `find_package(FlowForge)`.
- **What must NOT go here:** Target definitions, source file lists (those belong in the respective `CMakeLists.txt`).
- **Future modules:** New module targets are added to `src/CMakeLists.txt`, not here.
- **Architectural coupling prevention:** CMake modules are reusable snippets. Build logic is centralized, preventing copy-paste drift.

### `third_party/` — Vendored Dependencies

- **Why it exists:** Provides a designated location for vendored libraries if system packages or FetchContent are impractical.
- **What belongs here:** Vendored library source, with original `LICENSE` files.
- **What must NOT go here:** Random utility scripts, generated files.
- **Future modules:** Only if a dependency cannot be obtained otherwise (rare).
- **Architectural coupling prevention:** Vendoring is the exception, not the rule. This directory's existence discourages casual vendoring.

---

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
```

**Legend:**
- Solid arrows indicate `target_link_libraries` dependencies.
- A module may only depend on modules "below" it.
- No circular dependencies.

### Future Module Integration (No Circular Dependencies)

```
  Storage ──────────────► Task, DAG, Core
  Cache ─────────────────► Task, Core
  Docker ────────────────► Executor, Task, Core
  Distributed ───────────► Worker, Executor, Task, Core
  Logging ───────────────► Core (cross-cutting, injected everywhere)
  Metrics ───────────────► Core (cross-cutting, injected everywhere)
```

Future modules do NOT modify the existing dependency graph of v0.1 modules. They attach as new leaves or optional side branches.

---

## v0.1 Modules

These modules have full implementation in v0.1:

| Module | Directory | Purpose |
|--------|-----------|---------|
| **core** | `include/flowforge/core/`, `src/flowforge/core/` | Fundamental types (`TaskId`, `WorkflowId`), error handling (`Error`, `Expected`), C++23 concepts. Zero external dependencies. |
| **task** | `include/flowforge/task/`, `src/flowforge/task/` | `Task` class, `TaskConfig` (timeout, retries), `TaskResult`, `TaskStatus` enum. |
| **dag** | `include/flowforge/dag/`, `src/flowforge/dag/` | `DAG`, `Node`, `Edge`, topological sort, cycle detection. |
| **scheduler** | `include/flowforge/scheduler/`, `src/flowforge/scheduler/` | `IScheduler` interface, `TopologicalScheduler` implementation. |
| **executor** | `include/flowforge/executor/`, `src/flowforge/executor/` | `IExecutor` interface, `SyncExecutor` (synchronous, in-process). |
| **worker** | `include/flowforge/worker/`, `src/flowforge/worker/` | `WorkerPool`, `Worker`, bounded MPMC `TaskQueue`. |
| **runtime** | `include/flowforge/runtime/`, `src/flowforge/runtime/` | `Runtime` orchestrator: loads DAG, schedules, executes via workers. |
| **cli** | `include/flowforge/cli/`, `src/flowforge/cli/` | CLI application with `run` and `status` subcommands. |
| **logging** | `include/flowforge/logging/`, `src/flowforge/logging/` | `ILogger` interface + `ConsoleLogger`. Minimal observability for v0.1. |

---

## Future Modules

These directories exist with placeholder documentation but contain **no compiled code** in v0.1:

| Module | Directory | Planned Version | Reason for Deferral |
|--------|-----------|-----------------|---------------------|
| **storage** | `include/flowforge/storage/`, `src/flowforge/storage/` | v0.3+ | Requires PostgreSQL schema design and libpqxx integration. Needs stable task output contracts first. |
| **cache** | `include/flowforge/cache/`, `src/flowforge/cache/` | v0.4+ | BLAKE3 content-addressable cache depends on stable task output semantics. |
| **docker** | `include/flowforge/docker/`, `src/flowforge/docker/` | v0.5+ | Docker isolation requires container lifecycle management and Docker SDK/subprocess orchestration. |
| **distributed** | `include/flowforge/distributed/`, `src/flowforge/distributed/` | v0.6+ | Distributed workers require network protocol, message queue, and worker discovery. Major architectural addition. |
| **metrics** | `include/flowforge/metrics/`, `src/flowforge/metrics/` | v0.2+ | Prometheus/metrics exposition can be added once runtime is stable. |

**Implementation rule for future modules:**
- The `include/` directory contains a `README.md` explaining the module's purpose.
- The `src/` directory exists but contains no files added to the build.
- When ready to implement, add source files to `src/flowforge/<module>/` and update `src/CMakeLists.txt`.

---

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

10. **Benchmarking Rule:** Any module with a hot path (DAG traversal, scheduling, worker queue) must have a corresponding benchmark. Benchmarks live in `benchmarks/` and run in CI on every PR.

11. **Documentation Rule:** Every public class and function has a Doxygen comment. Design decisions are recorded as DDRs in `docs/design/`.

12. **No External Dependency Leakage:** If FlowForge uses a third-party library (e.g., nlohmann/json, BLAKE3), the types are wrapped in FlowForge-specific types. External consumers never see third-party headers unless explicitly intended.

13. **Incremental Enablement:** Future modules exist as empty directories with placeholder documentation so the architecture is complete, but they are not added to the build in v0.1.

14. **Concurrency Safety:** Every public class documents its thread-safety guarantees: `@threadsafe`, `@not_threadsafe`, or `@threadsafe_after_init`.

---

## Internal Module Design

### Core

```
core/
├── concepts.hpp     # C++23 concepts: TaskConcept, ExecutorConcept, SchedulerConcept
├── error.hpp        # Error type, error categories, make_error()
├── expected.hpp     # Thin wrapper around std::expected with FlowForge utilities
├── id.hpp           # Strongly-typed IDs: TaskId, WorkflowId, ExecutionId
└── status.hpp       # Common status enums: TaskStatus, ExecutionPhase
```

**Dependencies:** None (foundation of the entire library).

### Task

```
task/
├── task.hpp           # Task class: name, command/function, config, metadata
├── task_config.hpp    # TaskConfig: timeout, retries, environment, resource limits
└── task_result.hpp    # TaskResult: exit code, stdout, stderr, duration, artifacts
```

**Dependencies:** Core.

### DAG

```
dag/
├── dag.hpp              # DAG class: owns nodes and edges, provides topological order
├── node.hpp             # Node: wraps a TaskId, adds node-level metadata
├── edge.hpp             # Edge: source TaskId, target TaskId
├── topological_sort.hpp # Kahn's algorithm / DFS-based sort
└── cycle_detection.hpp  # DFS-based cycle detection with path reporting
```

**Dependencies:** Core, Task.

### Scheduler

```
scheduler/
├── scheduler.hpp           # IScheduler interface: schedule() returns next batch of runnable tasks
└── topological_scheduler.hpp # Default implementation: priority-aware topological scheduling
```

**Dependencies:** Core, DAG, Task.

### Executor

```
executor/
├── executor.hpp          # IExecutor interface: execute(task, context)
├── execution_context.hpp # ExecutionContext: runtime state, working directory, env
└── sync_executor.hpp     # SyncExecutor: runs task synchronously in-process (v0.1)
```

**Dependencies:** Core, Task.

### Worker

```
worker/
├── worker.hpp       # Worker: owns a thread, pulls tasks from queue, executes via IExecutor
├── worker_pool.hpp  # WorkerPool: manages N workers, bounded task queue, shutdown
└── task_queue.hpp   # Bounded MPMC queue (lock-free or mutex-based)
```

**Dependencies:** Core, Executor.

### Runtime

```
runtime/
├── runtime.hpp           # Runtime: main user-facing orchestrator
├── workflow_runner.hpp   # WorkflowRunner: executes a single workflow instance
├── execution_state.hpp   # ExecutionState: tracks pending/running/completed/failed
└── events.hpp            # Event system: TaskStartedEvent, TaskCompletedEvent, TaskFailedEvent
```

**Dependencies:** Core, Task, DAG, Scheduler, Executor, Worker.

### Storage (future)

```
storage/
├── storage_backend.hpp        # IStorageBackend interface
├── postgresql_storage.hpp     # PostgreSQL implementation (libpqxx)
└── repositories/
    ├── workflow_repository.hpp
    ├── task_repository.hpp
    └── execution_repository.hpp
```

**Dependencies:** Core, Task, DAG.

### Cache (future)

```
cache/
├── cache_backend.hpp      # ICacheBackend interface
└── content_hash_cache.hpp # BLAKE3-based content-addressable cache
```

**Dependencies:** Core, Task.

### Docker (future)

```
docker/
└── docker_executor.hpp # DockerExecutor: executes tasks inside Docker containers
```

**Dependencies:** Core, Executor, Task.

### Distributed (future)

```
distributed/
├── distributed_worker_pool.hpp # Network-transparent worker pool
└── task_queue.hpp              # Network message queue for task dispatch
```

**Dependencies:** Core, Worker, Executor, Task.

### Logging

```
logging/
├── logger.hpp        # ILogger interface: log(Level, message)
└── console_logger.hpp # ConsoleLogger: human-readable console output (v0.1)
```

**Dependencies:** Core. Cross-cutting — injected everywhere.

### Metrics (future)

```
metrics/
├── metrics_collector.hpp    # IMetricsCollector interface
└── prometheus_exporter.hpp  # Prometheus text format exposition
```

**Dependencies:** Core. Cross-cutting — injected everywhere.

### CLI

```
cli/
├── cli_app.hpp          # CLI class: parses args, dispatches to commands
└── commands/
    ├── run_command.hpp  # `flowforge run <workflow>`
    └── status_command.hpp # `flowforge status <execution>`
```

**Dependencies:** Runtime.
