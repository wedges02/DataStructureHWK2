// geometry.h — Geometry primitives for polygon simplification
#pragma once

#include <algorithm>
#include <cmath>

struct Point
{
    double x, y;
    Point() : x(0), y(0) {}
    Point(double x_, double y_) : x(x_), y(y_) {}
};

inline double Cross(const Point &a, const Point &b, const Point &c)
{
    return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

inline double TriArea(const Point &a, const Point &b, const Point &c)
{
    return 0.5 * Cross(a, b, c);
}

// Proper segment intersection (endpoints touching doesn't count)
inline bool SegmentsProperlyIntersect(const Point &p1, const Point &p2,
                                      const Point &p3, const Point &p4)
{
    double d1 = Cross(p3, p4, p1);
    double d2 = Cross(p3, p4, p2);
    double d3 = Cross(p1, p2, p3);
    double d4 = Cross(p1, p2, p4);

    if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
        ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)))
    {
        return true;
    }

    auto onSeg = [](const Point &p, const Point &q, const Point &r) -> bool
    {
        return std::min(q.x, r.x) - 1e-12 <= p.x && p.x <= std::max(q.x, r.x) + 1e-12 &&
               std::min(q.y, r.y) - 1e-12 <= p.y && p.y <= std::max(q.y, r.y) + 1e-12;
    };
    if (std::fabs(d1) < 1e-12 && onSeg(p1, p3, p4)) return true;
    if (std::fabs(d2) < 1e-12 && onSeg(p2, p3, p4)) return true;
    if (std::fabs(d3) < 1e-12 && onSeg(p3, p1, p2)) return true;
    if (std::fabs(d4) < 1e-12 && onSeg(p4, p1, p2)) return true;

    return false;
}
