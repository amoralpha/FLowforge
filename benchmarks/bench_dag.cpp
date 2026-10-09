#include <benchmark/benchmark.h>

#include <string>

#include "flowforge/dag/DAG.hpp"

static void BM_DAG_Insert(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));

    for (auto _ : state) {
        flowforge::DAG dag;

        for (int i = 0; i < n; ++i) {
            dag.addTask("task_" + std::to_string(i), {});
        }

        benchmark::DoNotOptimize(dag);
    }
}

static void BM_DAG_Validate(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));

    flowforge::DAG dag;

    for (int i = 0; i < n; ++i) {
        dag.addTask("task_" + std::to_string(i), {});
    }

    for (auto _ : state) {
        auto result = dag.validate();
        benchmark::DoNotOptimize(result);
    }
}
static void BM_DAG_Validate_Chain(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));

    flowforge::DAG dag;

    dag.addTask("task_0", {});

    for (int i = 1; i < n; ++i) {
        dag.addTask(
            "task_" + std::to_string(i),
            {"task_" + std::to_string(i - 1)}
        );
    }

    for (auto _ : state) {
        auto result = dag.validate();
        benchmark::DoNotOptimize(result);
    }
}



BENCHMARK(BM_DAG_Insert)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

BENCHMARK(BM_DAG_Validate)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

BENCHMARK(BM_DAG_Validate_Chain)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

BENCHMARK_MAIN();