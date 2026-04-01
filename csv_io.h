// csv_io.h — CSV parsing and output for polygon simplification
#pragma once

#include "geometry.h"
#include "spatial_grid.h"
#include "apsc_core.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Parse input CSV into per-ring point lists.
// Expected columns: ring_id, vertex_id, x, y (with header row).
inline std::map<int, std::vector<Point>> ParseInputCSV(const std::string &path)
{
    std::ifstream fin(path);
    if (!fin.is_open())
    {
        std::cerr << "Cannot open " << path << "\n";
        return {};
    }
    
    std::map<int, std::vector<Point>> ringPoints;
    std::string line;
    std::getline(fin, line); // skip header
    while (std::getline(fin, line))
    {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string tok;
        std::getline(ss, tok, ','); int rid = std::stoi(tok);
        std::getline(ss, tok, ','); // vertex_id (unused)
        std::getline(ss, tok, ','); double x = std::stod(tok);
        std::getline(ss, tok, ','); double y = std::stod(tok);
        ringPoints[rid].push_back(Point(x, y));
    }
    fin.close();
    return ringPoints;
}

// Write output CSV and summary statistics.
inline void WriteOutput(const std::vector<RingInfo> &rings,
                        const std::vector<Vertex> &vertPool,
                        double totalInputArea,
                        double totalOutputArea,
                        double totalDisplacement)
{
    int numRings = (int)rings.size();
    std::cout << "ring_id,vertex_id,x,y\n";
    for (int r = 0; r < numRings; r++)
    {
        int vid = 0, start = rings[r].head, v = start;
        do
        {
            std::ostringstream xss, yss;
            xss << std::setprecision(10) << vertPool[v].pt.x;
            yss << std::setprecision(10) << vertPool[v].pt.y;
            std::cout << r << "," << vid++ << "," << xss.str() << "," << yss.str() << "\n";
            v = vertPool[v].next;
        } while (v != start);
    }

    std::printf("Total signed area in input: %e\n", totalInputArea);
    std::printf("Total signed area in output: %e\n", totalOutputArea);
    std::printf("Total areal displacement: %e\n", totalDisplacement);
}
