# Area- and Topology-Preserving Polygon Simplification

C++ implementation of the Area-Preserving Segment Collapse (APSC) algorithm from Kronenfeld et al. (2020) for CSD2183 Data Structures Project 2.

Given a polygon (with optional holes) and a target vertex count, the program iteratively collapses segments to reduce vertex count while exactly preserving the area of each ring and maintaining topological validity (no self-intersections, no ring crossings).

## Dependencies

- **C++17** compiler (g++ recommended)
- **Make**
- No third-party libraries required. The implementation uses only the C++ standard library.

On Ubuntu/WSL:
```
sudo apt install g++ make
```

On macOS:
```
xcode-select --install
```

## Building

```
make
```

This produces an executable called `simplify` in the repository root.

## Usage

```
./simplify <input_file.csv> <target_vertices>
```

- `input_file.csv` — CSV file with columns `ring_id,vertex_id,x,y`. Ring 0 is the exterior ring (counterclockwise); rings 1, 2, ... are holes (clockwise).
- `target_vertices` — desired maximum total vertex count across all rings.

Output is printed to stdout in the same CSV format, followed by three summary lines:
```
Total signed area in input: <scientific notation>
Total signed area in output: <scientific notation>
Total areal displacement: <scientific notation>
```

## Project Structure

```
simplify.cpp        Main APSC loop, vertex pool, priority queue orchestration
geometry.h          Point struct, cross product, triangle area, segment intersection
spatial_grid.h      Spatial hash grid for O(1)-amortized intersection queries
apsc_core.h         ComputeE placement, ComputeDisplacement, Collapse/RingInfo structs
csv_io.h            CSV parsing (ParseInputCSV) and output formatting (WriteOutput)
makefile            Build target + test harness with 15 test cases
test_cases/         Input CSVs, expected outputs, and custom test cases
```

## Key Data Structures

- **Circular doubly linked list** — each ring is stored as a circular linked list in a global vertex pool, enabling O(1) vertex removal and neighbor traversal during collapse.
- **Min-heap priority queue** (`std::priority_queue` with `std::greater`) — selects the collapse candidate with minimum areal displacement. Uses lazy deletion via version counters to avoid expensive re-heapification.
- **Spatial hash grid** (`std::unordered_map<int64_t, vector<int>>`) — accelerates intersection checks by bucketing edges into grid cells. Includes a brute-force fallback (MAX_CELLS cap) for edges that span too many cells, and periodic full rebuilds to adapt cell size as vertex count drops.

## Test Results

All 15 test cases pass. Area is preserved exactly (within floating-point tolerance) on every case. Run tests with:

```
make test
```

### Reference test cases (provided by instructors)

| Test Case | Vertices | Holes | Target | Result |
|---|---|---|---|---|
| rectangle_with_two_holes | 12 | 2 | 7 | PASS |
| cushion_with_hexagonal_hole | 22 | 1 | 13 | PASS |
| blob_with_two_holes | 36 | 2 | 17 | PASS |
| wavy_with_three_holes | 43 | 3 | 21 | PASS |
| lake_with_two_islands | 81 | 2 | 17 | PASS |
| original_01 | 1,860 | 0 | 99 | PASS |
| original_02 | 8,605 | 0 | 99 | PASS |
| original_03 | 74,559 | 0 | 99 | PASS |
| original_04 | 6,733 | 0 | 99 | PASS |
| original_05 | 6,230 | 0 | 99 | PASS |
| original_06 | 14,122 | 0 | 99 | PASS |
| original_07 | 10,596 | 0 | 99 | PASS |
| original_08 | 6,850 | 0 | 99 | PASS |
| original_09 | 409,998 | 0 | 99 | PASS |
| original_10 | 9,899 | 0 | 99 | PASS |

**Summary: 15 passed, 0 failed, 0 skipped out of 15**

All tests preserve area exactly. Some outputs differ from expected in final vertex positions due to tie-breaking order, but signed areas match.

### Custom test cases

Four additional test cases (`test_cases/junbo_test_*.csv`) were created to test specific scenarios:
- `junbo_test_star.csv` — star-shaped polygon with narrow spikes (stress-tests near-degenerate collapses)
- `junbo_test_blackhole.csv` — polygon with tightly nested holes (tests close ring proximity)
- `junbo_test_singapore.csv` — real-world geographic boundary (high vertex count)
- `junbo_test_sit.csv` — complex boundary with irregular features

## Reference

Kronenfeld, B. J., L. V. Stanislawski, B. P. Buttenfield, and T. Brockmeyer (2020). "Simplification of polylines by segment collapse: minimizing areal displacement while preserving area". *International Journal of Cartography* 6.1, pp. 22-46.
