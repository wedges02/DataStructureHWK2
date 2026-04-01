#include "../spatial_grid.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

std::vector<Vertex> pool;

static bool NearlyEqual(double a, double b, double eps = 1e-12)
{
    return std::fabs(a - b) <= eps;
}

int main()
{
    SpatialGrid grid;

    grid.Init(2.0, 5);
    assert(NearlyEqual(grid.invCellSize, 0.5));
    assert(grid.cells.empty());
    assert(grid.queryGen.size() == 5);
    assert(grid.curGen == 0);
    for (int v : grid.queryGen) assert(v == 0);

    int64_t k1 = grid.CellKey(1, 2);
    int64_t k2 = grid.CellKey(1, 2);
    int64_t k3 = grid.CellKey(2, 1);
    assert(k1 == k2);
    assert(k1 != k3);

    assert(grid.ToGrid(3.9) == 1);
    assert(grid.ToGrid(0.0) == 0);
    assert(grid.ToGrid(-0.1) == -1);

    Point a(0.0, 0.0), b(3.9, 0.0);
    assert(grid.CellSpan(a, b) == 2);

    Point c(0.0, 0.0), d(0.0, 4.1);
    assert(grid.CellSpan(c, d) == 3);

    Point e(-2.1, -2.1), f(2.1, 2.1);
    assert(grid.CellSpan(e, f) == 16);

    grid.cells[grid.CellKey(0, 0)].push_back(42);
    grid.queryGen[2] = 99;
    grid.curGen = 7;

    grid.Rebuild(1.0, 8);
    assert(NearlyEqual(grid.invCellSize, 1.0));
    assert(grid.cells.empty());
    assert(grid.queryGen.size() == 5);
    assert(grid.curGen == 0);
    for (int v : grid.queryGen) assert(v == 0);

    std::cout << "spatial_grid_helpers_lifecycle_tests: PASS\n";
    return 0;
}
