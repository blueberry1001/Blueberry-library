# Graph batch two: candidate research and measurements

2026-09-18. All production code and benchmark alternatives were independently implemented; linked sources are algorithm/API research, not copied code.

## Research and selection

- Cycle detection: [CP-Algorithms recursive DFS](https://cp-algorithms.com/graph/finding-cycle.html) uses three colors and parents; [its graph library](https://lib.cp-algorithms.com/cp-algo/graph/cycle.hpp.html) reconstructs through DFS callbacks. We use an explicit stack and incoming edge IDs, extending the simple-graph exposition to loops and parallel edges. Both linear-time designs need O(N+M) storage; recursion additionally risks call-stack exhaustion.
- Topological sort: [CP-Algorithms](https://cp-algorithms.com/graph/topological-sort.html) uses DFS finish times; [KACTL](https://raw.githubusercontent.com/kth-competitive-programming/kactl/main/content/graph/TopoSort.h) uses indegrees and a vector queue. Our API returns optional to distinguish an empty DAG from failure. Kahn/vector and Kahn/queue candidates both take O(N+M) time and O(N) extra memory.
- Tree diameter: [CP-Algorithms](https://lib.cp-algorithms.com/cp-algo/tree/diameter.hpp.html) propagates maximum upward paths with tree DP; [Nachia](https://nachiavivias.github.io/cp-library/cpp/tree/tree-diameter.html) exposes a unit-weight diameter vertex path. We choose nonnegative weighted two-pass traversal and path reconstruction; recursive and iterative versions are O(N) time/memory, but only the latter survives arbitrary path depth.
- Triangles: [hitonanode](https://hitonanode.github.io/cplib-cpp/other_algorithms/enumerate_triangles.hpp.html) and [maspypy](https://maspypy.github.io/library/enumerate/triangle.hpp) orient edges by (degree, ID) then mark neighboring vertices. The latter also deduplicates input. We require a simple graph and avoid that cost. O(N+M sqrt(M)) time/O(N+M) memory improves worst-case scalability over neighbor-pair enumeration with an O(N²) adjacency matrix.

## Reproduction

Run from repository root under WSL Ubuntu:

```sh
g++ -std=gnu++20 -O2 -DNDEBUG -I. benchmark/graph-batch-two.cpp -o /tmp/graph-batch-two-benchmark
/tmp/graph-batch-two-benchmark
```

Every candidate pair checks equal return checksums, and all five raw samples are retained. An earlier comparable run storing summaries only remains in `results/graph-batch-two-summary-only.txt`.

Raw environment and results: [results/graph-batch-two.txt](results/graph-batch-two.txt).
GCC 13.3.0, WSL2 Linux x86_64, identical flags, five serial runs per candidate, mt19937 seed 20260918. Candidate graph construction is included when required; input generation is excluded. No peak-RSS measurement was made; memory comparisons above are asymptotic only. Other verification work may run on this shared machine, so these small timing differences are not robust performance claims.

| Candidate | minimum ms | median ms |
|---|---:|---:|
| Cycle explicit stack | 3.381 | 3.585 |
| Cycle recursive | 2.632 | 2.885 |
| Topological vector queue | 0.623 | 0.634 |
| Topological std::queue | 0.586 | 0.626 |
| Diameter iterative | 0.393 | 0.405 |
| Diameter recursive | 0.490 | 0.530 |
| Triangle degree orientation | 2.498 | 2.544 |
| Triangle neighbor pairs + matrix | 1.632 | 1.652 |

DAG: N=20,000, M=99,997; weighted random recursive tree: N=20,000; simple random graph: N=1,800, M=65,296. The cycle benchmark is an acyclic input, so both candidates fully traverse it and produce no reconstruction. The recursive cycle candidate is faster here, but rejected for unbounded recursion depth. The matrix triangle candidate is faster on this small graph; it is rejected for quadratic memory and its unbounded-by-degree worst-case running time. This is not a claim that the adopted implementation wins every workload.

An initial exploratory run omitted matrix construction, so its timing is not comparable to the production candidate. Its observed medians were cycle 3.03061/2.55079, topo 0.576088/0.596725, diameter 0.435875/0.529602, triangles 2.63316/0.860323 ms (production/alternative). It is retained here only as an excluded measurement; the raw final log and table above include matrix construction.

## Validation

`tests/random/graph-batch-two.cpp` accepts a seed argument and reports the seed/trial before assertions. It checks directed cycles against transitive closure, undirected cycles against DSU, triangle lists against cubic enumeration, diameter length/path against all-pairs distances, topological existence/order against transitive closure, and 200,000-vertex paths. Singleton, empty, zero-weight, self-loop and parallel-edge cases are included. Official verify drivers cover directed/undirected cycle detection, tree diameter and triangle enumeration. Topological sort has no direct Library Checker task and is covered locally.
