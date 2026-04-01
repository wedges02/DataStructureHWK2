// spatial_grid.h — Spatial hash grid for fast edge intersection queries
#pragma once

#include "geometry.h"

#include <vector>
#include <unordered_map>
#include <cstdint>

// Forward declaration: the grid reads vertex positions from this pool
struct Vertex;  
extern std::vector<Vertex> pool;

struct SpatialGrid
{
    double invCellSize;
    std::unordered_map<int64_t, std::vector<int>> cells;
    std::vector<int> queryGen;
    int curGen;

    // max cells a single operation can touch before 
    // we give up and do a brute-force check
    static constexpr long long MAX_CELLS = 2000;
};

// Vertex defined after SpatialGrid so both can
// co-exist in header-only form
struct Vertex
{
    Point pt;
    int prev, next;
    int ringId;
    int version;
    bool alive;
};