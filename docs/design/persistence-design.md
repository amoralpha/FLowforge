# Persistence Design

## Problem

FlowForge needs to persist workflow definitions, execution state, and task results to support:

1. Workflow resumption after crashes.
2. Historical execution audit trails.
3. Distributed worker coordination (future).

## Decision

Use a **repository pattern** with pluggable storage backends.

### Architecture

```cpp
class IStorageBackend {
public:
    virtual ~IStorageBackend() = default;
    virtual Expected<void> save_workflow(const Workflow& workflow) = 0;
    virtual Expected<Workflow> load_workflow(WorkflowId id) = 0;
    virtual Expected<void> save_execution(const Execution& execution) = 0;
    virtual Expected<Execution> load_execution(ExecutionId id) = 0;
    virtual Expected<void> save_task_result(const TaskResult& result) = 0;
    virtual Expected<std::vector<TaskResult>> load_task_results(ExecutionId id) = 0;
};
```

### Repository Layer

Thin wrappers around `IStorageBackend` providing type-safe queries:

```cpp
class WorkflowRepository {
public:
    explicit WorkflowRepository(IStorageBackend& backend);
    Expected<WorkflowId> create(const Workflow& workflow);
    Expected<Workflow> find(WorkflowId id);
    Expected<std::vector<Workflow>> list();
private:
    IStorageBackend& backend_;
};
```

### PostgreSQL Implementation (v0.3+)

- Schema: `workflows`, `tasks`, `executions`, `task_results`.
- Use libpqxx for C++ bindings.
- Connection pooling via `std::shared_ptr<pqxx::connection>`.

### Alternatives Considered

- **SQLite:** Simple, but lacks concurrent writer support needed for distributed workers.
- **Flat files (JSON/MessagePack):** No query capability, no concurrent access control.
- **Key-value store (Redis):** Good for caching, not ideal for relational workflow data.

## Future Considerations

- Event sourcing: store all state transitions as immutable events.
- Snapshotting: periodic full-state snapshots for fast recovery.
- Migration system for schema evolution.
