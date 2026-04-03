// apsc_core.h — APSC placement and displacement functions (Kronenfeld et al. 2020)
#pragma once

#include "geometry.h"

#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// Line equation for the area-preserving line E
// ---------------------------------------------------------------------------
struct LinEq
{
    double a, b, c;
};

inline LinEq ComputeLineE(const Point &A, const Point &B,
                           const Point &C, const Point &D)
{
    LinEq eq;
    eq.a = D.y - A.y;
    eq.b = A.x - D.x;
    eq.c = -B.y * A.x + (A.y - C.y) * B.x + (B.y - D.y) * C.x + C.y * D.x;
    return eq;
}

inline bool LineECoincidentWithAD(const LinEq &eq, const Point &A,
                                  [[maybe_unused]] const Point &D)
{
    double cAD = -(eq.a * A.x + eq.b * A.y);
    double norm = std::sqrt(eq.a * eq.a + eq.b * eq.b);
    if (norm < 1e-30) return true;
    return std::fabs(eq.c - cAD) / norm < 1e-12;
}

inline bool IntersectLineEWithLine(const LinEq &eq, const Point &P,
                                   const Point &Q, Point &result)
{
    double a2 = Q.y - P.y;
    double b2 = P.x - Q.x;
    double c2 = a2 * P.x + b2 * P.y;
    double det = eq.a * b2 - a2 * eq.b;
    if (std::fabs(det) < 1e-20) return false;
    result.x = ((-eq.c) * b2 - c2 * eq.b) / det;
    result.y = (eq.a * c2 - a2 * (-eq.c)) / det;
    return true;
}

// ---------------------------------------------------------------------------
// Placement function (Figures 4-6)
// ---------------------------------------------------------------------------
inline Point ComputeE(const Point &A, const Point &B,
                      const Point &C, const Point &D)
{
    LinEq eq = ComputeLineE(A, B, C, D);

    if (LineECoincidentWithAD(eq, A, D))
    {
        return D; // Figure 6a tie-breaker
    }

    double cAD = -(eq.a * A.x + eq.b * A.y);
    double fB = eq.a * B.x + eq.b * B.y + cAD;
    double fC = eq.a * C.x + eq.b * C.y + cAD;
    double normAD = std::sqrt(eq.a * eq.a + eq.b * eq.b);
    double dB = std::fabs(fB) / normAD;
    double dC = std::fabs(fC) / normAD;

    bool sameSideAD;
    if (dB < 1e-12 || dC < 1e-12)
    {
        sameSideAD = true;
    }
    else
    {
        sameSideAD = (fB * fC > 0);
    }

    double fEOnAD = -(eq.c - cAD);
    bool useAB;

    if (sameSideAD)
    {
        if (std::fabs(dB - dC) < 1e-12 * std::max(1.0, dB + dC))
        {
            useAB = (dB <= dC + 1e-15); // Fig 6c tie-breaker
        }
        else
        {
            useAB = (dB > dC);
        }
    }
    else
    {
        useAB = (fB * fEOnAD > 0);
    }

    Point result;
    if (useAB)
    {
        if (IntersectLineEWithLine(eq, A, B, result)) return result;
        if (IntersectLineEWithLine(eq, C, D, result)) return result;
        return B;
    }
    else
    {
        if (IntersectLineEWithLine(eq, C, D, result)) return result;
        if (IntersectLineEWithLine(eq, A, B, result)) return result;
        return C;
    }
}

// ---------------------------------------------------------------------------
// Areal displacement between polyline A->B->C->D and polyline A->E->D
// ---------------------------------------------------------------------------
inline double ShoelaceArea(const Point *pts, int n)
{
    double area = 0.0;
    for (int i = 0; i < n; i++)
    {
        int j = (i + 1) % n;
        area += pts[i].x * pts[j].y - pts[j].x * pts[i].y;
    }
    return 0.5 * area;
}

inline bool PointOnSegment(const Point &P, const Point &S0, const Point &S1, double &t)
{
    double dx = S1.x - S0.x, dy = S1.y - S0.y, len2 = dx * dx + dy * dy;
    if (len2 < 1e-30) return false;
    t = ((P.x - S0.x) * dx + (P.y - S0.y) * dy) / len2;
    if (t < 1e-9 || t > 1.0 - 1e-9) return false;
    double cx = (P.x - S0.x) * dy - (P.y - S0.y) * dx;
    return std::fabs(cx) < std::sqrt(len2) * 1e-9;
}

struct OrderedPoint
{
    double gp;
    Point pt;
    bool operator<(const OrderedPoint &o) const { return gp < o.gp; }
};

inline void DedupOrderedPoints(OrderedPoint *arr, int &n)
{
    int j = 0;
    for (int i = 0; i < n; i++)
    {
        if (j == 0 || std::fabs(arr[i].gp - arr[j - 1].gp) > 1e-15)
            arr[j++] = arr[i];
    }
    n = j;
}

struct SharedPair
{
    int i1, i2;
    bool operator<(const SharedPair &o) const { return i1 < o.i1; }
};

inline double ComputeDisplacement(const Point &A, const Point &B,
                                  const Point &C, const Point &D,
                                  const Point &E)
{
    struct XPt { double gp1, gp2; Point pt; };
    XPt xpts[16];
    int nxp = 0;

    Point segs[5][2] = {{A,B},{B,C},{C,D},{A,E},{E,D}};
    int pairs[4][2] = {{0,4},{1,3},{1,4},{2,3}};
    for (int p = 0; p < 4; p++)
    {
        int i = pairs[p][0], j = pairs[p][1];
        double dx1 = segs[i][1].x - segs[i][0].x, dy1 = segs[i][1].y - segs[i][0].y;
        double dx2 = segs[j][1].x - segs[j][0].x, dy2 = segs[j][1].y - segs[j][0].y;
        double denom = dx1 * dy2 - dy1 * dx2;
        if (std::fabs(denom) < 1e-20) continue;
        double dx3 = segs[j][0].x - segs[i][0].x, dy3 = segs[j][0].y - segs[i][0].y;
        double t = (dx3 * dy2 - dy3 * dx2) / denom;
        double u = (dx3 * dy1 - dy3 * dx1) / denom;
        if (t > 1e-12 && t < 1.0 - 1e-12 && u > 1e-12 && u < 1.0 - 1e-12)
        {
            xpts[nxp++] = {(double)i + t, (double)(j - 3) + u,
                           {segs[i][0].x + t * dx1, segs[i][0].y + t * dy1}};
        }
    }

    double tv;
    if (PointOnSegment(B, A, E, tv)) xpts[nxp++] = {1.0, tv, B};
    if (PointOnSegment(B, E, D, tv)) xpts[nxp++] = {1.0, 1.0 + tv, B};
    if (PointOnSegment(C, A, E, tv)) xpts[nxp++] = {2.0, tv, C};
    if (PointOnSegment(C, E, D, tv)) xpts[nxp++] = {2.0, 1.0 + tv, C};
    if (PointOnSegment(E, A, B, tv)) xpts[nxp++] = {tv, 1.0, E};
    if (PointOnSegment(E, B, C, tv)) xpts[nxp++] = {1.0 + tv, 1.0, E};
    if (PointOnSegment(E, C, D, tv)) xpts[nxp++] = {2.0 + tv, 1.0, E};

    OrderedPoint p1[20], p2[20];
    int np1 = 0, np2 = 0;
    p1[np1++] = {0, A}; p1[np1++] = {1, B}; p1[np1++] = {2, C}; p1[np1++] = {3, D};
    p2[np2++] = {0, A}; p2[np2++] = {1, E}; p2[np2++] = {2, D};
    for (int i = 0; i < nxp; i++)
    {
        p1[np1++] = {xpts[i].gp1, xpts[i].pt};
        p2[np2++] = {xpts[i].gp2, xpts[i].pt};
    }
    std::sort(p1, p1 + np1);
    std::sort(p2, p2 + np2);
    DedupOrderedPoints(p1, np1);
    DedupOrderedPoints(p2, np2);

    SharedPair shared[20];
    int nsh = 0;
    shared[nsh++] = {0, 0};
    for (int x = 0; x < nxp; x++)
    {
        int i1 = -1, i2 = -1;
        for (int k = 0; k < np1; k++)
        {
            if (std::fabs(p1[k].gp - xpts[x].gp1) < 1e-12) { i1 = k; break; }
        }
        for (int k = 0; k < np2; k++)
        {
            if (std::fabs(p2[k].gp - xpts[x].gp2) < 1e-12) { i2 = k; break; }
        }
        if (i1 >= 0 && i2 >= 0) shared[nsh++] = {i1, i2};
    }
    shared[nsh++] = {np1 - 1, np2 - 1};
    std::sort(shared, shared + nsh);
    {
        int j = 0;
        for (int i = 0; i < nsh; i++)
        {
            if (j == 0 || shared[i].i1 != shared[j - 1].i1)
                shared[j++] = shared[i];
        }
        nsh = j;
    }

    double totalDisp = 0.0;
    for (int s = 0; s + 1 < nsh; s++)
    {
        Point sub[20];
        int ns = 0;
        for (int k = shared[s].i1; k <= shared[s + 1].i1; k++) sub[ns++] = p1[k].pt;
        for (int k = shared[s + 1].i2 - 1; k > shared[s].i2; k--) sub[ns++] = p2[k].pt;
        if (ns >= 3) totalDisp += std::fabs(ShoelaceArea(sub, ns));
    }
    return totalDisp;
}

// Collapse candidate
struct Collapse
{
    int A, B, C, D;
    int versionA, versionB, versionC, versionD;
    Point E;
    double displacement;
    int seqId;

    bool operator>(const Collapse &o) const
    {
        if (displacement != o.displacement) return displacement > o.displacement;
        return seqId > o.seqId;
    }
};

// Ring bookkeeping
struct RingInfo
{
    int ringId;
    int size;
    int head;
};
