# Ownership and Lifetime

## 1. DAG Ownership

The DAG owns its internal graph data.

The graph is represented using:
- `dependencies_`: task IDs mapped to their dependencies.
- `dependents_`: task IDs mapped to tasks that depend on them.

Task IDs and adjacency lists are stored by value using
`std::string`, `std::vector`, and `std::unordered_map`.

The DAG does not need callers to keep the original strings
or dependency vectors alive after `addTask()` returns.

## 2. Reference-Returning Queries

The following methods return const references:

- `getDependencies(const std::string& id)`
- `getDependents(const std::string& id)`

These references do not own the returned vectors.
The vectors are managed by the DAG.

Rules:
- Do not use a returned reference after its owning DAG
  has been destroyed, moved, or assigned.
- Do not assume query results remain unchanged while the
  DAG is still being constructed.
- Do not modify the returned vectors through these const
  references.

Querying an unknown task throws `std::out_of_range`.

## 3. DAG Mutation and Validation

A successful call to `validate()` freezes the DAG.

After successful validation, calling `addTask()` throws
`std::invalid_argument`.

Validation errors do not represent a successful freeze.
For example, a missing dependency can be added before
validation is successfully completed.

## 4. Validation Result Ownership

`validate()` returns `std::expected<TopoOrder, DagError>`
by value.

The returned topological order and error data own their
stored strings and vectors. They do not borrow those
containers from the DAG.

## 5. Concurrency

This API does not promise thread-safe concurrent access.
Callers must synchronize concurrent operations when
necessary, especially if the DAG is being modified.