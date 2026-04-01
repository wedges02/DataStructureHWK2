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

    int64_t CellKey(int ix, int iy) const
    {
        return ((int64_t)ix << 32) | (int64_t)(unsigned int)iy;
    }

    int ToGrid(double v) const
    {
        return (int)std::floor(v * invCellSize);
    }

    long long CellSpan(const Point &a, const Point &b) const
    {
        int x0 = ToGrid(std::min(a.x, b.x)), x1 = ToGrid(std::max(a.x, b.x));
        int y0 = ToGrid(std::min(a.y, b.y)), y1 = ToGrid(std::max(a.y, b.y));
        
        return (long long)(x1 - x0 + 1) * (y1 - y0 + 1);
    }

    void Init(double cs, int numVerts)
    {
        invCellSize = 1.0 / cs;
        cells.clear();
        cells.reserve(numVerts * 2);
        queryGen.assign(numVerts, 0);
        curGen = 0;
    }

    void Rebuild(double cs, int approxEdges)
    {
        invCellSize = 1.0 / cs;
        cells.clear();
        cells.reserve(approxEdges * 2);
        curGen = 0;
        std::fill(queryGen.begin(), queryGen.end(), 0);
    }
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