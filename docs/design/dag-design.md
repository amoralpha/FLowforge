# DAG Design

## 1. Purpose

The Directed Acyclic Graph (DAG) module is the foundational data structure for FlowForge's workflow representation. It models workflows as graphs where vertices are tasks and directed edges represent dependency relationships. The DAG module is responsible solely for graph construction, validation, cycle detection, topological ordering, and immutability enforcement. It has no knowledge of execution, scheduling, workers, or persistence.

## 2. Scope / v0.1 Context

In v0.1, the DAG module provides the minimal contract required by the scheduler and runtime:

- Construction via `addTask(id, dependencies)`
- Deferred validation via `validate()`
- Topological ordering for execution planning

No persistence, serialization, or advanced graph algorithms are included in v0.1. The representation is designed to remain stable as the rest of FlowForge matures.

## 3. DAG Terminology

- **Task**: A unit of work identified by a human-readable string ID. Tasks are the vertices of the graph.
- **Dependency**: A directed edge from task A to task B meaning "A depends on B" or "B must complete before A can run."
- **Dependent**: The inverse of a dependency. If A depends on B, then B is a dependent of A.
- **In-degree**: The number of incoming edges to a task (the number of tasks it depends on).
- **Topological order**: A linear ordering of tasks such that for every directed edge (A → B), task A appears before task B in the ordering.
- **Cycle**: A path that starts and ends at the same task. Cycles are invalid in a workflow DAG.

## 4. Task Representation

In v0.1, a task is represented by its identifier alone. The task ID is a human-readable string. Minimal metadata is associated with tasks at this layer; task configuration, command definitions, and execution semantics belong to the `Task` module, which the DAG references only by ID.

This separation ensures the DAG remains a pure graph structure without coupling to execution details.

## 5. Construction API

The primary construction API is a single method:

```
addTask(id, dependencies)
```

- `id` is a human-readable string identifying the task.
- `dependencies` is a collection of task ID strings that this task depends on.
- The task and its dependencies are added to the graph atomically in one call.

**Duplicate rejection:** Adding a task with an `id` that already exists in the graph is a hard error. The operation must be rejected with a detailed error indicating the duplicate task ID.

**Forward references:** `addTask()` performs no dependency validation. Dependencies may reference task IDs that have not yet been added to the graph. This is explicitly supported to allow workflows to be constructed in any order.

Example of valid forward references:

```
addTask("A", {"B"})
addTask("B", {})
```

Here, "A" references "B" before "B" is added. This is permitted because validation is deferred.

**Self-loops:** Self-loops (a task depending on itself) are not specially handled during `addTask()`. They are treated as normal edges and will be detected later by the cycle-detection mechanism during `validate()`.

```
addTask("A", {"A"})   // Permitted during construction; fails validation
```

## 6. Deferred Validation Model

Validation is explicitly deferred. The DAG supports an incremental construction phase followed by an explicit validation phase.

**Construction phase:**
- `addTask()` calls are accepted.
- No dependency existence checks are performed.
- Forward references are allowed.
- The graph may be invalid (missing dependencies, cycles).

**Validation phase:**
- `validate()` is called explicitly when construction is complete.
- All validity checks run at this point.
- On success, the DAG freezes and becomes immutable.
- On failure, a detailed error is returned and the DAG remains mutable for correction.

This model separates graph construction from graph verification, allowing workflows to be built in natural, non-topological order.

## 7. Internal Representation

The DAG uses an adjacency-list style representation storing both directions of each edge.

Two maps are maintained:

**dependencies:**
- Maps a task ID to the set of task IDs it depends on.
- Example: `dependencies["A"] = {"B", "C"}` means A depends on B and C.

**dependents:**
- Maps a task ID to the set of task IDs that depend on it.
- Example: `dependents["B"] = {"A"}` means A depends on B.

For the example where A depends on B and C:

```
dependencies:
  A → {B, C}

dependents:
  B → {A}
  C → {A}
```

**Why both directions are maintained:**

- **Dependency queries:** Given a task, quickly determine what it depends on (used for validation and cycle detection).
- **Dependent queries:** Given a task, quickly determine what depends on it (used by the scheduler for readiness calculations and by Kahn's algorithm for in-degree updates).
- **O(1) lookup in both directions:** Both maps provide constant-time access to outgoing and incoming edges without recomputation.

Both maps are updated atomically when an edge is added. The internal representation is an implementation detail; external consumers interact only through the public API.

## 8. DAG Invariants

During the construction phase (before `validate()` is called):

- Task IDs are unique.
- `dependencies` and `dependents` maps are kept in sync.
- Forward references are permitted.
- The graph may contain missing dependencies or cycles.

After successful `validate()`:

- Every dependency reference resolves to an existing task.
- The graph is acyclic.
- A valid topological order exists and is stored.
- The graph is frozen; `addTask()` is rejected.

## 9. Validation Pipeline

`validate()` executes four sequential stages:

1. Missing dependency check
2. DFS cycle detection
3. Topological ordering (Kahn's algorithm)
4. Freeze the DAG

All four stages must succeed for validation to pass. If any stage fails, validation aborts and returns the corresponding error. The DAG remains mutable so the caller can correct issues and re-validate.

Stage 1 and Stage 2 are checks. Stage 3 computes a result used by later stages. Stage 4 is a state transition.

## 10. Missing Dependency Validation

**Purpose:** Ensure that every dependency edge references a task that exists in the graph.

**Algorithm:** Iterate over all entries in the `dependencies` map. For each task, iterate over its dependency set. Verify that every dependency ID exists as a key in the `dependencies` map (i.e., has been added via `addTask()`).

**Error detail:** If a missing dependency is found, the error must identify:
- The task that references the missing dependency.
- The missing dependency ID.

Example error:

```
Task "A" references missing dependency "Z".
```

Forward references are therefore legal during construction but must be resolved before `validate()` succeeds.

## 11. DFS Cycle Detection

**Purpose:** Detect cycles in the graph and report the cycle path to the user.

**Algorithm:** Depth-first search (DFS) with three-color marking:

- **WHITE**: Task has not been visited.
- **GRAY**: Task is currently being explored (present in the current DFS recursion stack).
- **BLACK**: Task has been fully processed.

**Cycle detection logic:** During DFS, when exploring an edge from the current task to a neighbor:
- If the neighbor is WHITE, recurse into it.
- If the neighbor is GRAY, a back-edge exists, indicating a cycle.
- If the neighbor is BLACK, the edge leads to an already-processed node and is ignored.

**Cycle path reporting:** When a back-edge is detected, the current DFS recursion path from the GRAY node back to itself is extracted and included in the error. This gives the user an understandable cycle path such as `A → B → C → A`.

**Self-loops:** A self-loop (A → A) is detected naturally by this mechanism. When exploring A, the edge to A encounters a GRAY node, triggering cycle detection with path `A → A`.

## 12. Topological Sorting with Kahn's Algorithm

**Purpose:** Produce a valid execution ordering of all tasks after the graph is confirmed to be acyclic.

**Algorithm:** Kahn's algorithm (BFS-based, in-degree driven).

**Definitions:**
- **In-degree**: The number of incoming edges to a task (the number of tasks it depends on).

**Steps:**
1. Compute the in-degree of every task by counting incoming edges from the `dependencies` map (or equivalently, outgoing edges from the `dependents` map).
2. Initialize a queue with all tasks whose in-degree is zero. These tasks have no unsatisfied dependencies and are ready to execute first.
3. Repeatedly remove a task from the queue and append it to the topological order.
4. For each task removed, decrease the in-degree of all tasks that depend on it (its dependents in the `dependents` map).
5. If any dependent's in-degree drops to zero, add it to the queue.
6. Continue until the queue is empty.

**Why Kahn's algorithm:** It explicitly constructs a valid topological ordering by greedily selecting zero-in-degree nodes. The resulting order respects all dependency constraints: if task A depends on task B, B will appear before A in the output.

**Why not DFS for topological sort:** DFS-based topological sort is a valid alternative, but Kahn's algorithm is chosen for its explicit in-degree tracking, which aligns naturally with the scheduler's readiness calculations. The separation of cycle detection (DFS) from topological ordering (Kahn's) is an intentional architectural decision described in Section 7.

## 13. DAG Freezing / Immutability

After all four validation stages succeed, the DAG transitions to a frozen state:

- `addTask()` is rejected going forward. Any attempt to modify the graph after validation returns an error.
- The computed topological order is stored and made available to consumers.
- The `dependencies` and `dependents` maps become read-only.

**Purpose:** The scheduler and runtime require a stable, validated graph during execution. Freezing guarantees that the topological order used for scheduling cannot be invalidated by concurrent or subsequent modifications. It also signals to consumers that the graph has passed all validity checks.

Before successful validation, the DAG is mutable. After successful validation, it is immutable.

## 14. Error Reporting

All validation errors must be actionable and specific:

- **Duplicate task ID:** Identify the duplicate ID and the point of insertion.
- **Missing dependency:** Identify the dependent task and the missing dependency ID.
- **Cycle detected:** Provide the full cycle path as a sequence of task IDs.

Errors are returned as structured error types from the `validate()` call. They must not be thrown as exceptions across module boundaries.

## 15. Complexity Discussion

Let V be the number of tasks and E be the number of dependency edges.

**Construction (`addTask`):**
- Amortized O(1) per task insertion (hash map insert).
- O(D) per task for D dependencies (map lookups and inserts).
- Total construction cost: O(V + E).

**Missing dependency check (Stage 1):**
- Iterates over all edges once.
- Time: O(V + E).

**DFS cycle detection (Stage 2):**
- Visits each task and edge exactly once.
- Time: O(V + E).
- Space: O(V) for the color map and recursion stack.

**Kahn's algorithm (Stage 3):**
- Computes in-degrees: O(E).
- Processes each task and edge once: O(V + E).
- Total: O(V + E).

**Overall validation:** O(V + E) time, O(V + E) space for the internal maps.

The adjacency-list representation with both-direction maps ensures that dependency queries, dependent queries, and in-degree updates all operate in constant or linear time relative to edge count, with no unnecessary recomputation.

## 16. Relationship to the FlowForge Scheduler

The DAG module provides validated, immutable graph data to the Scheduler. The Scheduler consumes:

- The topological order to understand execution precedence.
- The `dependents` map to determine which tasks become ready when a task completes.
- The `dependencies` map to verify scheduler state against graph structure.

The DAG does not schedule tasks, manage workers, execute commands, or handle retries. Those responsibilities belong to the Scheduler, Executor, and Worker modules. The DAG's boundary is the validated, frozen graph contract.

## 17. Testing Considerations

The DAG module must be tested at the following levels:

- **Construction tests:** Verify `addTask()` accepts valid inputs, rejects duplicate IDs, and preserves forward references.
- **Validation tests:** Verify `validate()` detects missing dependencies, cycles (including self-loops), and succeeds on valid graphs.
- **Cycle detection tests:** Verify specific cycle patterns are detected and reported with correct paths.
- **Topological sort tests:** Verify Kahn's algorithm produces valid orderings for known DAGs.
- **Freeze tests:** Verify `addTask()` is rejected after successful `validate()`.
- **Error message tests:** Verify errors contain the expected task IDs and cycle paths.

Test fixtures should include small, hand-verifiable DAGs with known topological orders and known cycle structures.

## 18. Summary of Design Decisions

| Decision | Rationale |
|----------|-----------|
| Task IDs are human-readable strings | Matches workflow definition formats; simplifies debugging and user-facing error messages. |
| `addTask(id, dependencies)` as primary API | Allows natural, non-topological workflow construction. |
| Duplicate IDs rejected immediately | Prevents ambiguous graph state; fail-fast on construction errors. |
| Deferred validation via explicit `validate()` | Separates construction from verification; supports forward references and flexible build order. |
| Forward references allowed during construction | Workflows can be defined in any order; validation ensures completeness before execution. |
| Self-loops treated as normal edges | Simpler construction logic; naturally caught by general cycle detection without special casing. |
| Both `dependencies` and `dependents` maps | Enables O(1) bidirectional lookup; supports scheduler readiness calculations and Kahn's in-degree updates. |
| Four-stage validation pipeline | Clear separation of concerns: missing dependency checks, cycle detection, ordering, and freezing. |
| DFS (white/gray/black) for cycle detection | Standard, reliable cycle detection with path reporting for actionable errors. |
| Kahn's algorithm for topological ordering | Explicit in-degree tracking aligns with scheduler semantics; deterministic and easy to verify. |
| Cycle detection and topological sort are separate steps | DFS answers "does a cycle exist and what is it?" while Kahn's answers "what is a valid order?" — distinct responsibilities. |
| DAG freezes after successful validation | Guarantees graph stability for downstream consumers; prevents mutation during execution. |
| No scheduler/executor responsibilities in DAG | Maintains single responsibility; the DAG is a pure graph abstraction. |
