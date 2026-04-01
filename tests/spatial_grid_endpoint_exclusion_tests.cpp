#include "../spatial_grid.h"

#include <cassert>
#include <iostream>
#include <vector>

std::vector<Vertex> pool;

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
    grid.AddEdge(1);
    grid.AddEdge(2);
    grid.AddEdge(3);

    // Query touches polygon only at corner (0, 0).
    assert(grid.FindIntersection(Point(-1.0, 0.0), Point(0.0, 0.0), -1, -1, -1, -1));

    // Excluding vertex 0 suppresses both incident edges (0->1 and 3->0).
    assert(!grid.FindIntersection(Point(-1.0, 0.0), Point(0.0, 0.0), 0, -1, -1, -1));

    // Exclusion behavior should match even when forced into fallback scan.
    assert(!grid.FindIntersection(Point(-100.0, 0.0), Point(0.0, 0.0), 0, -1, -1, -1));

    std::cout << "spatial_grid_endpoint_exclusion_tests: PASS\n";
    return 0;
}
