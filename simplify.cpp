// Implementation of the APSC algorithm from Kronenfeld et al. (2020)

#include "geometry.h"
#include "spatial_grid.h"
#include "apsc_core.h"
#include "csv_io.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <vector>

// Global vertex pool (referenced by spatial_grid.h)
std::vector<Vertex> pool;

static SpatialGrid grid;
static bool useGrid = true;

using CollapseQueue = std::priority_queue<Collapse, std::vector<Collapse>, std::greater<Collapse>>;

static double ComputeRingArea(const std::vector<RingInfo> &rings, int rid)
{
    if (rings[rid].size < 3) return 0.0;
    double area = 0.0;
    int start = rings[rid].head;
    int v = start;
    do
    {
        int nxt = pool[v].next;
        area += pool[v].pt.x * pool[nxt].pt.y - pool[nxt].pt.x * pool[v].pt.y;
        v = nxt;
    } while (v != start);
    return area * 0.5;
}

static Collapse MakeCollapse(int ai, int bi, int ci, int di, int &seqCounter)
{
    Collapse col;
    col.A = ai; col.B = bi; col.C = ci; col.D = di;
    col.versionA = pool[ai].version;
    col.versionB = pool[bi].version;
    col.versionC = pool[ci].version;
    col.versionD = pool[di].version;
    col.E = ComputeE(pool[ai].pt, pool[bi].pt, pool[ci].pt, pool[di].pt);
    col.displacement = ComputeDisplacement(pool[ai].pt, pool[bi].pt, pool[ci].pt, pool[di].pt, col.E);
    col.seqId = seqCounter++;
    return col;
}

static bool BruteForceIntersection(const Point &P, const Point &Q, int exA, int exB, int exC, int exD)
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

static bool SegIntersectsAny(const Point &P, const Point &Q, int exA, int exB, int exC, int exD)
{
    if (!useGrid) return BruteForceIntersection(P, Q, exA, exB, exC, exD);
    return grid.FindIntersection(P, Q, exA, exB, exC, exD);
}

static void DoRebuild(CollapseQueue &pq, std::vector<RingInfo> &rings, double gridSpan, int totalVertices, int &seqCounter, int &collapsesSinceRebuild)
{
    while (!pq.empty()) pq.pop();

    if (useGrid)
    {
        double cs = gridSpan / (4.0 * std::sqrt((double)std::max(totalVertices, 4)));
        if (cs < 1e-12) cs = 1.0;
        grid.Rebuild(cs, totalVertices);

        for (int r = 0; r < (int)rings.size(); r++)
        {
            if (rings[r].size < 4) continue;
            int start = rings[r].head;
            int v = start;
            do
            {
                grid.AddEdge(v);
                v = pool[v].next;
            } while (v != start);
        }
    }

    for (int r = 0; r < (int)rings.size(); r++)
    {
        if (rings[r].size < 4) continue;
        int start = rings[r].head;
        int ai = start;
        do
        {
            int bi = pool[ai].next;
            int ci = pool[bi].next;
            int di = pool[ci].next;
            pq.push(MakeCollapse(ai, bi, ci, di, seqCounter));
            ai = pool[ai].next;
        } while (ai != start);
    }

    collapsesSinceRebuild = 0;
}

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4)
    {
        std::cerr << "Usage: " << argv[0] << " <input.csv> <target_vertices> [--no-grid]\n";
        return 1;
    }

    std::string inputFile = argv[1];
    int targetVertices = std::atoi(argv[2]);
    if (argc == 4 && std::string(argv[3]) == "--no-grid") useGrid = false;

    // Parse input
    auto ringPoints = ParseInputCSV(inputFile);
    if (ringPoints.empty()) return 1;

    // Build vertex pool and circular linked lists
    int numRings = (int)ringPoints.size();
    std::vector<RingInfo> rings(numRings);
    int totalVertices = 0;

    for (auto &kv : ringPoints)
    {
        int rid = kv.first;
        auto &pts = kv.second;
        int n = (int)pts.size();
        rings[rid].ringId = rid;
        rings[rid].size = n;
        totalVertices += n;

        int base = (int)pool.size();
        rings[rid].head = base;
        for (int i = 0; i < n; i++)
        {
            Vertex v;
            v.pt = pts[i];
            v.ringId = rid;
            v.alive = true;
            v.version = 0;
            v.prev = base + ((i - 1 + n) % n);
            v.next = base + ((i + 1) % n);
            pool.push_back(v);
        }
    }

    // Compute input area
    double totalInputArea = 0.0;
    for (int r = 0; r < numRings; r++) totalInputArea += ComputeRingArea(rings, r);

    // Early exit
    if (totalVertices <= targetVertices)
    {
        double outArea = totalInputArea;
        WriteOutput(rings, pool, totalInputArea, outArea, 0.0);
        return 0;
    }

    // Initialize spatial grid
    double gridSpan = 0.0;
    {
        double minX = 1e300, maxX = -1e300, minY = 1e300, maxY = -1e300;
        for (auto &v : pool)
        {
            minX = std::min(minX, v.pt.x); maxX = std::max(maxX, v.pt.x);
            minY = std::min(minY, v.pt.y); maxY = std::max(maxY, v.pt.y);
        }
        gridSpan = std::max(maxX - minX, maxY - minY);
        if (useGrid)
        {
            double cs = gridSpan / (4.0 * std::sqrt((double)totalVertices));
            if (cs < 1e-12) cs = 1.0;
            grid.Init(cs, (int)pool.size());
            for (int r = 0; r < numRings; r++)
            {
                int start = rings[r].head;
                int v = start;
                do
                {
                    grid.AddEdge(v);
                    v = pool[v].next;
                } while (v != start);
            }
        }
    }

    // Build priority queue
    int seqCounter = 0;
    CollapseQueue pq;

    for (int r = 0; r < numRings; r++)
    {
        if (rings[r].size < 4) continue;
        int start = rings[r].head;
        int ai = start;
        do
        {
            int bi = pool[ai].next;
            int ci = pool[bi].next;
            int di = pool[ci].next;
            pq.push(MakeCollapse(ai, bi, ci, di, seqCounter));
            ai = pool[ai].next;
        } while (ai != start);
    }

    // Main APSC loop
    double totalDisplacement = 0.0;
    int collapsesSinceRebuild = 0;

    while (totalVertices > targetVertices && !pq.empty())
    {
        Collapse col = pq.top();
        pq.pop();

        // Validate: check versions match (lazy deletion)
        if (!pool[col.B].alive || !pool[col.C].alive) continue;
        if (!pool[col.A].alive || !pool[col.D].alive) continue;
        if (pool[col.A].version != col.versionA ||
            pool[col.B].version != col.versionB ||
            pool[col.C].version != col.versionC ||
            pool[col.D].version != col.versionD) continue;
        if (pool[col.B].prev != col.A || pool[col.B].next != col.C ||
            pool[col.C].next != col.D) continue;

        int rid = pool[col.B].ringId;
        if (rings[rid].size <= 3) continue;

        // Topology check
        int prevA = pool[col.A].prev;
        int nextD = pool[col.D].next;
        bool intersects = false;
        if (SegIntersectsAny(pool[col.A].pt, col.E, prevA, col.A, col.B, col.C))
        {
            intersects = true;
        }
        if (!intersects)
        {
            if (SegIntersectsAny(col.E, pool[col.D].pt, col.B, col.C, col.D, nextD))
            {
                intersects = true;
            }
        }
        if (intersects) continue;

        // Update spatial grid: remove old edges
        if (useGrid)
        {
            Point oldB = pool[col.B].pt;
            Point oldC = pool[col.C].pt;
            grid.RemoveEdge(col.A, pool[col.A].pt, oldB);
            grid.RemoveEdge(col.B, oldB, oldC);
            grid.RemoveEdge(col.C, oldC, pool[col.D].pt);
        }

        // Perform collapse: B becomes E, C is removed
        pool[col.B].pt = col.E;
        pool[col.B].version++;
        pool[col.C].alive = false;
        pool[col.B].next = col.D;
        pool[col.D].prev = col.B;

        if (rings[rid].head == col.C) rings[rid].head = col.B;

        // Add new edges to grid
        if (useGrid)
        {
            grid.AddEdge(col.A);
            grid.AddEdge(col.B);
        }

        rings[rid].size--;
        totalVertices--;
        totalDisplacement += col.displacement;
        collapsesSinceRebuild++;

        // Add new candidates
        int Ei = col.B;
        int Ai = col.A;
        int Di = col.D;
        if (rings[rid].size >= 4)
        {
            pq.push(MakeCollapse(pool[Ai].prev, Ai, Ei, Di, seqCounter));
            pq.push(MakeCollapse(Ai, Ei, Di, pool[Di].next, seqCounter));
            if (pool[pool[Ai].prev].prev != Ei)
            {
                pq.push(MakeCollapse(pool[pool[Ai].prev].prev, pool[Ai].prev, Ai, Ei, seqCounter));
            }
            if (pool[pool[Di].next].next != Ei)
            {
                pq.push(MakeCollapse(Ei, Di, pool[Di].next, pool[pool[Di].next].next, seqCounter));
            }
        }

        // Periodic rebuild to keep PQ and grid bounded as vertex count drops.
        if (collapsesSinceRebuild >= std::max(1000, totalVertices / 10))
        {
            DoRebuild(pq, rings, gridSpan, totalVertices, seqCounter, collapsesSinceRebuild);
        }
    }

    // Output
    double totalOutputArea = 0.0;
    for (int r = 0; r < numRings; r++) totalOutputArea += ComputeRingArea(rings, r);
    WriteOutput(rings, pool, totalInputArea, totalOutputArea, totalDisplacement);

    return 0;
}
