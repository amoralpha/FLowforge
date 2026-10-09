#pragma once

#include <expected>
#include <string>
#include <vector>
#include <unordered_map>

#include "flowforge/task/Task.hpp"

namespace flowforge {

enum class DagErrorKind {
    MissingDependency,
    CycleDetected
};

struct DagError {
    DagErrorKind kind;
    std::string taskId;
    std::vector<std::string> details;
};

using TopoOrder = std::vector<std::string>;

class DAG {
public:
    DAG() = default;
      //Sstring bs isliye because task id should be human-readable
    void addTask(const std::string& id,
        const std::vector<std::string>& dependencies
    );

    std::expected<TopoOrder, DagError> validate();

    const std::vector<std::string>& getDependencies(const std::string& id) const;
    const std::vector<std::string>& getDependents(const std::string& id) const;

private:

    std::unordered_map<std::string, std::vector<std::string>> dependencies_;
    std::unordered_map<std::string, std::vector<std::string>> dependents_;
    bool frozen_ = false;
    //Ye bahut important design decision hai because Scheduler ko eventually stable DAG chahiye.

    std::expected<void, DagError> checkMissingDependencies() const;
    std::expected<void, DagError> detectCycles() const;
    TopoOrder topologicalSort() const;
};

std::string toString(const DagError& error);

} // namespace flowforge
