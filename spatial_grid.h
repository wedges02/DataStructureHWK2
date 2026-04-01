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

    // Implemented after Vertex is fully defined.
    inline void AddEdge(int vi);
    
    void RemoveEdge(int vi, const Point &a, const Point &b)
    {
        int x0 = ToGrid(std::min(a.x, b.x)), x1 = ToGrid(std::max(a.x, b.x));
        int y0 = ToGrid(std::min(a.y, b.y)), y1 = ToGrid(std::max(a.y, b.y));

        if (CellSpan(a, b) > MAX_CELLS) return;

        for (int ix = x0; ix <= x1; ix++)
        {
            for (int iy = y0; iy <= y1; iy++)
            {
                auto it = cells.find(CellKey(ix, iy));

                if (it == cells.end()) continue;

                auto &bucket = it->second;

                for (int k = (int)bucket.size() - 1; k >= 0; k--)
                {
                    if (bucket[k] == vi)
                    {
                        bucket[k] = bucket.back();
                        bucket.pop_back();
                        break;
                    }
                }
            }
        }
    }

    // Check if segment P→Q intersects any live edge, excluding vertices in the
    // given set. Returns true on first intersection found.
    inline bool FindIntersection(const Point &P, const Point &Q,
                                 int exA, int exB, int exC, int exD);
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

inline void SpatialGrid::AddEdge(int vi)
{
    const Point &a = pool[vi].pt;
    const Point &b = pool[pool[vi].next].pt;
    int x0 = ToGrid(std::min(a.x, b.x)), x1 = ToGrid(std::max(a.x, b.x));
    int y0 = ToGrid(std::min(a.y, b.y)), y1 = ToGrid(std::max(a.y, b.y));

    if (CellSpan(a, b) > MAX_CELLS) return;

    for (int ix = x0; ix <= x1; ix++)
    {
        for (int iy = y0; iy <= y1; iy++)
        {
            cells[CellKey(ix, iy)].push_back(vi);
        }
    }
}


inline bool SpatialGrid::FindIntersection(const Point &P, const Point &Q,
                                          int exA, int exB, int exC, int exD)
{
    curGen++;

    int x0 = ToGrid(std::min(P.x, Q.x)), x1 = ToGrid(std::max(P.x, Q.x));
    int y0 = ToGrid(std::min(P.y, Q.y)), y1 = ToGrid(std::max(P.y, Q.y));

    if (CellSpan(P, Q) > MAX_CELLS)
    {
        for (int vi = 0; vi < (int)pool.size(); vi++)
        {
            if (!pool[vi].alive) continue;
            
            int nxt = pool[vi].next;

            if (vi == exA || vi == exB || vi == exC || vi == exD ||
                nxt == exA || nxt == exB || nxt == exC || nxt == exD) continue;

            if (SegmentsProperlyIntersect(P, Q, pool[vi].pt, pool[nxt].pt)) return true;
        }

        return false;
    }

    for (int ix = x0; ix <= x1; ix++)
    {
        for (int iy = y0; iy <= y1; iy++)
        {
            auto it = cells.find(CellKey(ix, iy));
            if (it == cells.end()) continue;

            for (int vi : it->second)
            {
                if (queryGen[vi] == curGen) continue;
                queryGen[vi] = curGen;

                if (!pool[vi].alive) continue;
                int nxt = pool[vi].next;

                if (vi == exA || vi == exB || vi == exC || vi == exD ||
                    nxt == exA || nxt == exB || nxt == exC || nxt == exD)
                {
                    continue;
                }

                if (SegmentsProperlyIntersect(P, Q, pool[vi].pt, pool[nxt].pt))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

