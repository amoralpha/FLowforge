# Getting Started with FlowForge

This guide walks you through building FlowForge from source and running a simple workflow.

## Prerequisites

- **Compiler:** GCC 13+, Clang 17+, or MSVC 19.35+ (C++23 support required)
- **CMake:** 3.28 or later
- **Ninja:** Recommended generator
- **Git:** For cloning and submodules

## Build

```bash
# Clone the repository
git clone https://github.com/your-org/flowforge.git
cd flowforge

# Configure (development build with tests and benchmarks)
cmake --preset dev -G Ninja

# Build
cmake --build build/dev

# Run tests
ctest --test-dir build/dev

# Run the CLI
./build/dev/bin/flowforge --help
```

## First Workflow

Create a file `workflow.json`:

```json
{
  "name": "hello-world",
  "tasks": [
    { "id": "greet", "command": "echo Hello" },
    { "id": "world", "command": "echo World", "depends_on": ["greet"] }
  ]
}
```

Run it:

```bash
./build/dev/bin/flowforge run workflow.json
```

## Next Steps

- Read the [Architecture](architecture.md) to understand the codebase.
- Explore [Examples](../examples/) for more workflow patterns.
- Read [Design Documents](design/) for deep dives into specific subsystems.
