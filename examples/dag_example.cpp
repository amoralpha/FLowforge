#include <iostream>
#include "flowforge/dag/DAG.hpp"

int main() {
    std::cout << "=== FlowForge DAG Core Example ===\n\n";

    // --- Valid DAG ---
    //   A -> B
    //   A -> C
    //   B -> D
    //   C -> D
    {
        flowforge::DAG dag;

        dag.addTask("A", {"B", "C"});
        dag.addTask("B", {"D"});
        dag.addTask("C", {"D"});
        dag.addTask("D", {});

        auto result = dag.validate();
        if (result) {
            std::cout << "Valid DAG: validation succeeded.\n";
            std::cout << "Topological order: ";
            for (const auto& task : *result) {
                std::cout << task << " ";
            }
            std::cout << "\n";
        } else {
            std::cout << "Valid DAG: validation failed — " << flowforge::toString(result.error()) << "\n";
        }
    }

    std::cout << "\n---\n\n";

    // --- Invalid DAG (cycle) ---
    //   A -> B -> C -> A
    {
        flowforge::DAG dag;

        dag.addTask("A", {"B"});
        dag.addTask("B", {"C"});
        dag.addTask("C", {"A"});

        auto result = dag.validate();
        if (result) {
            std::cout << "Invalid DAG: validation succeeded (unexpected).\n";
        } else {
            std::cout << "Invalid DAG: validation failed — " << flowforge::toString(result.error()) << "\n";
        }
    }

    std::cout << "\n---\n\n";

    // --- Forward reference example ---
    {
        flowforge::DAG dag;

        dag.addTask("A", {"B"});
        dag.addTask("B", {});

        auto result = dag.validate();
        if (result) {
            std::cout << "Forward reference DAG: validation succeeded.\n";
            std::cout << "Topological order: ";
            for (const auto& task : *result) {
                std::cout << task << " ";
            }
            std::cout << "\n";
        } else {
            std::cout << "Forward reference DAG: validation failed — " << flowforge::toString(result.error()) << "\n";
        }
    }

    return 0;
}
