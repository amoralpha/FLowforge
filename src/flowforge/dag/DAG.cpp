#include "flowforge/dag/DAG.hpp"

#include <algorithm>
#include <queue>
#include <stdexcept>

namespace flowforge {

void DAG::addTask(const std::string& id, const std::vector<std::string>& dependencies) {
    if (frozen_) {
        throw std::invalid_argument(
            "Cannot add task to a frozen DAG. Call validate() only after construction is complete."
        );
    }

    if (dependencies_.contains(id)) {
        throw std::invalid_argument("Duplicate task ID: " + id);
    }

    // Deduplicate dependencies while preserving insertion order.
    // The internal representation stores vectors, so duplicates would corrupt
    // in-degree calculations in Kahn's algorithm.
    std::vector<std::string> unique_deps;
    unique_deps.reserve(dependencies.size());

    for (const auto& dep : dependencies) {
        if (std::find(unique_deps.begin(), unique_deps.end(), dep) == unique_deps.end()) {
            unique_deps.push_back(dep);
        }
    }

    dependencies_[id] = std::move(unique_deps);
    dependents_[id]; // Ensure an entry exists for tasks with no dependents.

    for (const auto& dep : dependencies_[id]) {
        dependents_[dep].push_back(id);
    }
}

std::expected<TopoOrder, DagError> DAG::validate() {
    // Stage 1: Missing dependency check.
    auto missing = checkMissingDependencies();
    if (!missing) {
        return std::unexpected{missing.error()};
    }

    // Stage 2: DFS cycle detection.
    auto cycle = detectCycles();
    if (!cycle) {
        return std::unexpected{cycle.error()};
    }

    // Stage 3: Topological ordering via Kahn's algorithm.
    TopoOrder order = topologicalSort();

    // Stage 4: Freeze the DAG.
    frozen_ = true;

    return order;
}

std::expected<void, DagError> DAG::checkMissingDependencies() const {
    for (const auto& [task_id, deps] : dependencies_) {
        for (const auto& dep : deps) {
            if (!dependencies_.contains(dep)) {
                return std::unexpected{
                    DagError{
                        DagErrorKind::MissingDependency,
                        task_id,
                        {dep}
                    }
                };
            }
        }
    }
    return {};
}

std::expected<void, DagError> DAG::detectCycles() const {
    enum class Color { WHITE, GRAY, BLACK };

    std::unordered_map<std::string, Color> color;
    std::unordered_map<std::string, std::string> parent;

    for (const auto& [id, _] : dependencies_) {
        color[id] = Color::WHITE;
    }

    for (const auto& [start_id, _] : dependencies_) {
        if (color[start_id] != Color::WHITE) {
            continue;
        }

        // Iterative DFS with explicit stack.
        // Each stack entry holds the current node and the index of the next
        // neighbor to visit, simulating the recursive call frame.
        std::vector<std::pair<std::string, size_t>> stack;
        stack.emplace_back(start_id, 0);
        parent[start_id] = "";

        while (!stack.empty()) {
            auto& [current, next_idx] = stack.back();

            if (color[current] == Color::WHITE) {
                color[current] = Color::GRAY;
            }

            auto it = dependencies_.find(current);
            if (it == dependencies_.end() || next_idx >= it->second.size()) {
                color[current] = Color::BLACK;
                stack.pop_back();
                continue;
            }

            const std::string& neighbor = it->second[next_idx];
            next_idx++;

            auto color_it = color.find(neighbor);
            if (color_it == color.end()) {
                continue; // Should not occur after Stage 1.
            }

            if (color_it->second == Color::GRAY) {
                // Back-edge found: neighbor is an ancestor in the current DFS path.
                if (current == neighbor) {
                    // Self-loop.
                    return std::unexpected{
                        DagError{
                            DagErrorKind::CycleDetected,
                            current,
                            {current, current}
                        }
                    };
                }

                // Reconstruct the cycle path from neighbor through current back to neighbor.
                std::vector<std::string> cycle_path;
                std::string node = current;
                while (node != neighbor) {
                    cycle_path.push_back(node);
                    node = parent.at(node);
                }
                cycle_path.push_back(neighbor);
                std::reverse(cycle_path.begin(), cycle_path.end());
                cycle_path.push_back(neighbor); // Close the cycle.

                return std::unexpected{
                    DagError{
                        DagErrorKind::CycleDetected,
                        neighbor,
                        std::move(cycle_path)
                    }
                };
            } else if (color_it->second == Color::WHITE) {
                parent[neighbor] = current;
                stack.emplace_back(neighbor, 0);
            }
        }
    }

    return {};
}

TopoOrder DAG::topologicalSort() const {
    std::unordered_map<std::string, size_t> in_degree;
    std::queue<std::string> ready;

    // Initialize in-degrees from the dependency map.
    for (const auto& [task_id, deps] : dependencies_) {
        in_degree[task_id] = deps.size();
        if (in_degree[task_id] == 0) {
            ready.push(task_id);
        }
    }

    TopoOrder order;
    order.reserve(dependencies_.size());

    while (!ready.empty()) {
        std::string current = ready.front();
        ready.pop();
        order.push_back(current);

        // Decrement in-degree of every task that depends on the current task.
        auto it = dependents_.find(current);
        if (it != dependents_.end()) {
            for (const auto& dependent : it->second) {
                in_degree[dependent]--;
                if (in_degree[dependent] == 0) {
                    ready.push(dependent);
                }
            }
        }
    }

    return order;
}

const std::vector<std::string>& DAG::getDependencies(const std::string& id) const {
    auto it = dependencies_.find(id);
    if (it == dependencies_.end()) {
        throw std::out_of_range("Task not found in DAG: " + id);
    }
    return it->second;
}

const std::vector<std::string>& DAG::getDependents(const std::string& id) const {
    auto it = dependents_.find(id);
    if (it == dependents_.end()) {
        throw std::out_of_range("Task not found in DAG: " + id);
    }
    return it->second;
}

std::string toString(const DagError& error) {
    switch (error.kind) {
        case DagErrorKind::MissingDependency:
            return "Task '" + error.taskId +
                   "' references missing dependency '" +
                   error.details.at(0) + "'.";

        case DagErrorKind::CycleDetected: {
            std::string result = "Cycle detected:\n";

            for (size_t i = 0; i < error.details.size(); ++i) {
                if (i > 0) {
                    result += " -> ";
                }

                result += error.details[i];
            }

            return result;
        }

        default:
            return "Unknown error";
    }
}
} // namespace flowforge
