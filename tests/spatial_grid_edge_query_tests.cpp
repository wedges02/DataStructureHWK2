#include "../spatial_grid.h"

#include <cassert>
#include <iostream>
#include <vector>

std::vector<Vertex> pool;

static bool Contains(const std::vector<int> &v, int x)
{
    for (int e : v)
    {
        if (e == x) return true;
    }
    return false;
}

int main()
{
    SpatialGrid grid;
    grid.Init(1.0, 4);

    pool.assign(4, Vertex{});
    pool[0] = {Point(0.0, 0.0), 3, 1, 0, 0, true};
    pool[1] = {Point(4.0, 0.0), 0, 2, 0, 0, true};
    pool[2] = {Point(4.0, 4.0), 1, 3, 0, 0, true};
    pool[3] = {Point(0.0, 4.0), 2, 0, 0, 0, true};

    grid.AddEdge(0);
    for (int x = 0; x <= 4; x++)
    {
        auto it = grid.cells.find(grid.CellKey(x, 0));
        assert(it != grid.cells.end());
        assert(Contains(it->second, 0));
    }

    grid.RemoveEdge(0, pool[0].pt, pool[1].pt);
    for (int x = 0; x <= 4; x++)
    {
        auto it = grid.cells.find(grid.CellKey(x, 0));
        if (it == grid.cells.end()) continue;
        assert(!Contains(it->second, 0));
    }

    grid.AddEdge(0);
    grid.AddEdge(1);
    grid.AddEdge(2);
    grid.AddEdge(3);

    assert(grid.FindIntersection(Point(-1.0, 2.0), Point(5.0, 2.0), -1, -1, -1, -1));
    assert(!grid.FindIntersection(Point(-1.0, -1.0), Point(5.0, -1.0), -1, -1, -1, -1));

    assert(!grid.FindIntersection(Point(-1.0, 2.0), Point(5.0, 2.0), 0, 1, 2, 3));

    assert(grid.FindIntersection(Point(-100.0, 2.0), Point(100.0, 2.0), -1, -1, -1, -1));

    std::cout << "spatial_grid_edge_query_tests: PASS\n";
    return 0;
}
