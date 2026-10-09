# Storage Module

**Status:** Future — not implemented in v0.1.

This module will provide persistence backends for workflows, tasks, and execution state.

Planned components:
- `IStorageBackend` — abstract storage interface
- `PostgreSQLStorage` — PostgreSQL implementation (requires libpqxx)
- `repositories/` — typed repositories for workflows, tasks, executions

When implemented, `Runtime` will optionally accept an `IStorageBackend` for durable execution.

## Dependencies
- Core
- Task
- DAG

## v0.1 Boundary
No files compiled. Directory exists to reserve namespace and signal architectural intent.
