#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <unordered_map>

#include "flowforge/dag/DAG.hpp"

using namespace flowforge;


TEST_CASE("Empty DAG") {
    DAG dag;
    auto result = dag.validate();
    REQUIRE(result.has_value());
    REQUIRE(result->empty());
}

TEST_CASE("Single task") {
    DAG dag;
    dag.addTask("A", {});
    auto result = dag.validate();
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 1);
    REQUIRE(result->at(0) == "A");
}

TEST_CASE("Linear DAG: A -> B -> C") {
    DAG dag;
    dag.addTask("A", {"B"});
    dag.addTask("B", {"C"});
    dag.addTask("C", {});

    auto result = dag.validate();
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 3);
    REQUIRE(result->at(0) == "C");
    REQUIRE(result->at(1) == "B");
    REQUIRE(result->at(2) == "A");
}

TEST_CASE("Branching DAG: A -> B, A -> C") {
    DAG dag;
    dag.addTask("A", {"B", "C"});
    dag.addTask("B", {});
    dag.addTask("C", {});

    auto result = dag.validate();
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 3);

    std::unordered_map<std::string, size_t> pos;
    for (size_t i = 0; i < result->size(); ++i) {
        pos[(*result)[i]] = i;
    }

    REQUIRE(pos["B"] < pos["A"]);
    REQUIRE(pos["C"] < pos["A"]);
}

TEST_CASE("Merging DAG: B -> D, C -> D") {
    DAG dag;
    dag.addTask("D", {"B", "C"});
    dag.addTask("B", {});
    dag.addTask("C", {});

    auto result = dag.validate();
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 3);

    std::unordered_map<std::string, size_t> pos;
    for (size_t i = 0; i < result->size(); ++i) {
        pos[(*result)[i]] = i;
    }

    REQUIRE(pos["B"] < pos["D"]);
    REQUIRE(pos["C"] < pos["D"]);
}

TEST_CASE("Disconnected DAG") {
    DAG dag;
    dag.addTask("A", {});
    dag.addTask("B", {});

    auto result = dag.validate();
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 2);
}

TEST_CASE("Duplicate task ID") {
    DAG dag;
    dag.addTask("A", {});
    REQUIRE_THROWS_AS(dag.addTask("A", {}), std::invalid_argument);
}

TEST_CASE("Duplicate dependency edge") {
    DAG dag;
    dag.addTask("A", {"B", "B", "C"});
    dag.addTask("B", {});
    dag.addTask("C", {});

    auto result = dag.validate();
    REQUIRE(result.has_value());

    const auto& deps = dag.getDependencies("A");
    REQUIRE(deps.size() == 2);
    REQUIRE(std::find(deps.begin(), deps.end(), "B") != deps.end());
    REQUIRE(std::find(deps.begin(), deps.end(), "C") != deps.end());
}

TEST_CASE("Missing dependency") {
    DAG dag;
    dag.addTask("A", {"B"});

    auto result = dag.validate();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().kind == DagErrorKind::MissingDependency);
    REQUIRE(result.error().taskId == "A");
    REQUIRE(result.error().details == std::vector<std::string>{"B"});
}

TEST_CASE("Self-loop") {
    DAG dag;
    dag.addTask("A", {"A"});

    auto result = dag.validate();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().kind == DagErrorKind::CycleDetected);
    REQUIRE(result.error().taskId == "A");
    REQUIRE(result.error().details == std::vector<std::string>{"A", "A"});
}

TEST_CASE("Simple cycle: A -> B -> C -> A") {
    DAG dag;
    dag.addTask("A", {"B"});
    dag.addTask("B", {"C"});
    dag.addTask("C", {"A"});

    auto result = dag.validate();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().kind == DagErrorKind::CycleDetected);
    REQUIRE(result.error().details.size() >= 3);
    // Verify the cycle path starts and ends with the same task.
    REQUIRE(result.error().details.front() == result.error().details.back());
}

TEST_CASE("Larger cycle: A -> B -> C -> D -> A") {
    DAG dag;
    dag.addTask("A", {"B"});
    dag.addTask("B", {"C"});
    dag.addTask("C", {"D"});
    dag.addTask("D", {"A"});

    auto result = dag.validate();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().kind == DagErrorKind::CycleDetected);
    REQUIRE(result.error().details.size() == 5);
    REQUIRE(result.error().details.front() == result.error().details.back());
}

TEST_CASE("Valid topological order respects dependencies") {
    DAG dag;
    dag.addTask("A", {"B", "C"});
    dag.addTask("B", {"D"});
    dag.addTask("C", {"D"});
    dag.addTask("D", {});

    auto result = dag.validate();
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 4);

    std::unordered_map<std::string, size_t> pos;
    for (size_t i = 0; i < result->size(); ++i) {
        pos[(*result)[i]] = i;
    }

    REQUIRE(pos["D"] < pos["B"]);
    REQUIRE(pos["D"] < pos["C"]);
    REQUIRE(pos["B"] < pos["A"]);
    REQUIRE(pos["C"] < pos["A"]);
}

TEST_CASE("Query dependencies") {
    DAG dag;
    dag.addTask("A", {"B", "C"});
    dag.addTask("B", {});
    dag.addTask("C", {});

    const auto& a_deps = dag.getDependencies("A");
    REQUIRE(a_deps.size() == 2);
    REQUIRE(std::find(a_deps.begin(), a_deps.end(), "B") != a_deps.end());
    REQUIRE(std::find(a_deps.begin(), a_deps.end(), "C") != a_deps.end());

    const auto& b_deps = dag.getDependencies("B");
    REQUIRE(b_deps.empty());
}

TEST_CASE("Query dependents") {
    DAG dag;
    dag.addTask("A", {"B"});
    dag.addTask("B", {});

    const auto& b_dependents = dag.getDependents("B");
    REQUIRE(b_dependents.size() == 1);
    REQUIRE(b_dependents[0] == "A");

    const auto& a_dependents = dag.getDependents("A");
    REQUIRE(a_dependents.empty());
}

TEST_CASE("Query unknown task throws") {
    DAG dag;
    dag.addTask("A", {});
    REQUIRE_THROWS_AS(dag.getDependencies("Z"), std::out_of_range);
    REQUIRE_THROWS_AS(dag.getDependents("Z"), std::out_of_range);
}

TEST_CASE("Freeze behavior after successful validation") {
    DAG dag;
    dag.addTask("A", {});

    auto result = dag.validate();
    REQUIRE(result.has_value());

    REQUIRE_THROWS_AS(dag.addTask("B", {}), std::invalid_argument);
}

TEST_CASE("Forward references") {
    DAG dag;
    dag.addTask("A", {"B"});
    dag.addTask("B", {});

    auto result = dag.validate();
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 2);

    std::unordered_map<std::string, size_t> pos;
    for (size_t i = 0; i < result->size(); ++i) {
        pos[(*result)[i]] = i;
    }

    REQUIRE(pos["B"] < pos["A"]);
}

TEST_CASE("Multiple valid topological orders") {
    DAG dag;
    dag.addTask("A", {});
    dag.addTask("B", {});
    dag.addTask("C", {"A", "B"});

    auto result = dag.validate();
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 3);

    std::unordered_map<std::string, size_t> pos;
    for (size_t i = 0; i < result->size(); ++i) {
        pos[(*result)[i]] = i;
    }

    // Both A and B must come before C, but their relative order is unconstrained.
    REQUIRE(pos["A"] < pos["C"]);
    REQUIRE(pos["B"] < pos["C"]);
}
