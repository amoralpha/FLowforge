# Scheduler Design

## Problem

Given a DAG of tasks and their dependencies, the scheduler must determine which tasks are ready to execute at any point during workflow execution.

## Decision

Implement a **priority-aware topological scheduler** with a pluggable interface.

### Interface

```cpp
class IScheduler {
public:
    virtual ~IScheduler() = default;
    virtual std::vector<TaskId> schedule(const DAG& dag,
                                         const ExecutionState& state) = 0;
};
```

### Default Implementation: TopologicalScheduler

- Maintains in-degree counts for each node.
- Returns all tasks with in-degree == 0 that are not yet completed.
- Supports task priorities (higher priority tasks are scheduled first within the same batch).
- After a task completes, decrements in-degree of its dependents and re-evaluates.

### Scheduling Policy

```
1. Identify all tasks with no pending dependencies.
2. Filter out already-completed or currently-running tasks.
3. Sort by priority (descending), then by task ID (ascending) for determinism.
4. Return the sorted batch.
```

### Alternatives Considered

- **Simple FIFO queue:** No priority support, insufficient for production.
- **Critical path scheduling:** Computes longest path, but adds complexity. Defer to v0.5.
- **DAG-aware work-stealing:** Interesting for distributed workers, defer to distributed module.

## Future Considerations

- Critical path scheduling for deadline-aware workflows.
- Fair scheduling (prevent starvation of low-priority tasks).
- Preemption support for long-running tasks.
