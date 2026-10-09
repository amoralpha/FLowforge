# Third-Party Dependencies

This directory is reserved for **vendored** dependencies — libraries copied directly into the repository.

## Vendoring Policy

FlowForge prefers obtaining dependencies via one of the following methods (in order of preference):

1. **System package manager** (apt, brew, vcpkg, Conan, etc.)
2. **CMake FetchContent** with pinned versions (preferred for header-only or small libraries)
3. **Vendoring** (last resort, only if the above are impractical)

If vendoring is necessary:
- The vendored code must retain its original license in a `LICENSE` file within the subdirectory.
- The vendored version must be pinned to a specific release/tag.
- A `README.md` in this directory must explain why vendoring was chosen.
- Vendored code should not be modified unless a patch is strictly required (documented).

## Current Vendored Dependencies

None. All dependencies are obtained via CMake FetchContent or system packages.

## Adding a New Vendored Dependency

1. Create a subdirectory: `third_party/<library-name>/`
2. Copy the library source into it.
3. Add a `LICENSE` file from the upstream project.
4. Update this `README.md` with the library name, version, and justification.
5. Update `cmake/Dependencies.cmake` to use the vendored path.
