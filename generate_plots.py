#!/usr/bin/env python3
"""Generate experimental evaluation plots for the APSC project."""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import curve_fit

# ── Data ──────────────────────────────────────────────────────────────────────

# Shared vertex counts (sorted by size)
vertices = np.array([1860, 6230, 6733, 6850, 8605, 9899, 10596, 14122, 74559, 409998])

# (a) Running time — with spatial grid
times_grid = np.array([0.01, 0.04, 0.04, 0.04, 0.06, 0.06, 0.08, 0.11, 0.83, 5.70])

# (a) Running time — brute force (no grid); original_09 omitted (would timeout)
vertices_bf = np.array([1860, 6230, 6733, 6850, 8605, 9899, 10596, 14122, 74559])
times_bf    = np.array([0.04, 0.47, 0.58, 0.59, 0.89, 1.18, 1.42, 2.41, 65.30])

# (b) Peak memory — with spatial grid
memory_grid = np.array([4292, 4640, 5776, 5476, 5792, 5940, 6336, 6516, 22768, 87352])

# (b) Peak memory — brute force
memory_bf = np.array([4052, 4532, 4816, 4944, 5412, 5384, 5476, 5720, 19676])

# (c) Areal displacement vs target vertex count (original_02, 8605 input vertices)
targets = np.array([50, 100, 200, 500, 1000, 2000, 5000, 8000])
displacement = np.array([3.044635e+07, 1.477592e+07, 6.867519e+06, 2.226944e+06,
                         7.678988e+05, 2.182126e+05, 1.482933e+04, 1.458373e+02])

# ── Styling ───────────────────────────────────────────────────────────────────

plt.rcParams.update({
    'figure.facecolor': 'white',
    'axes.facecolor': 'white',
    'axes.grid': True,
    'grid.alpha': 0.3,
    'font.size': 12,
    'figure.dpi': 150,
})

BLUE = '#2563eb'
RED = '#dc2626'
GREEN = '#16a34a'
ORANGE = '#f97316'
GRAY = '#6b7280'

# ── Fit functions ─────────────────────────────────────────────────────────────

def nlogn(n, c):
    return c * n * np.log(n)

def n_squared(n, c):
    return c * n * n

def linear(n, c, b):
    return c * n + b

# ── Plot (a): Running Time vs Input Size ──────────────────────────────────────

popt_grid, _ = curve_fit(nlogn, vertices, times_grid, p0=[1e-7])
popt_bf, _   = curve_fit(n_squared, vertices_bf, times_bf, p0=[1e-9])

fig, ax = plt.subplots(figsize=(9, 6))

# Fit curves
x_fit = np.linspace(1500, 450000, 500)
ax.plot(x_fit, nlogn(x_fit, popt_grid[0]), color=BLUE, linewidth=1.5, linestyle='--', alpha=0.6,
        label=f'Fit: {popt_grid[0]:.2e} \u00b7 n ln(n)')
x_fit_bf = np.linspace(1500, 80000, 500)
ax.plot(x_fit_bf, n_squared(x_fit_bf, popt_bf[0]), color=RED, linewidth=1.5, linestyle='--', alpha=0.6,
        label=f'Fit: {popt_bf[0]:.2e} \u00b7 n\u00b2')

# Data points
ax.scatter(vertices, times_grid, color=BLUE, s=60, zorder=5, label='With spatial grid')
ax.scatter(vertices_bf, times_bf, color=RED, s=60, zorder=5, marker='x', linewidths=2, label='Brute force (no grid)')

ax.set_xscale('log')
ax.set_yscale('log')
ax.set_xlabel('Input Vertices (n)')
ax.set_ylabel('Running Time (seconds)')
ax.set_title('Running Time vs. Input Size')
ax.legend(fontsize=10)
fig.tight_layout()
fig.savefig('plots/running_time_vs_input_size.png')
plt.close(fig)

# ── Plot (b): Peak Memory vs Input Size ───────────────────────────────────────

popt_mg, _ = curve_fit(linear, vertices, memory_grid, p0=[0.2, 3000])
popt_mb, _ = curve_fit(linear, vertices_bf, memory_bf, p0=[0.2, 3000])

fig, ax = plt.subplots(figsize=(9, 6))

# Fit curves
x_fit = np.linspace(1500, 450000, 500)
ax.plot(x_fit, linear(x_fit, *popt_mg), color=GREEN, linewidth=1.5, linestyle='--', alpha=0.6,
        label=f'Grid fit: {popt_mg[0]:.2f} \u00b7 n + {popt_mg[1]:.0f} KB')
x_fit_bf = np.linspace(1500, 80000, 500)
ax.plot(x_fit_bf, linear(x_fit_bf, *popt_mb), color=ORANGE, linewidth=1.5, linestyle='--', alpha=0.6,
        label=f'Brute force fit: {popt_mb[0]:.2f} \u00b7 n + {popt_mb[1]:.0f} KB')

# Data points
ax.scatter(vertices, memory_grid, color=GREEN, s=60, zorder=5, label='With spatial grid')
ax.scatter(vertices_bf, memory_bf, color=ORANGE, s=60, zorder=5, marker='x', linewidths=2, label='Brute force (no grid)')

ax.set_xscale('log')
ax.set_yscale('log')
ax.set_xlabel('Input Vertices (n)')
ax.set_ylabel('Peak Memory (KB)')
ax.set_title('Peak Memory Usage vs. Input Size')
ax.legend(fontsize=10)
fig.tight_layout()
fig.savefig('plots/peak_memory_vs_input_size.png')
plt.close(fig)

# ── Plot (c): Areal Displacement vs Target Vertex Count ──────────────────────

fig, ax = plt.subplots(figsize=(9, 6))
ax.plot(targets, displacement, color=BLUE, marker='o', markersize=7, linewidth=2,
        label='original_02 (8605 input vertices)')
ax.set_yscale('log')
ax.set_xlabel('Target Vertex Count')
ax.set_ylabel('Total Areal Displacement')
ax.set_title('Areal Displacement vs. Target Vertex Count')
ax.legend(fontsize=10)
fig.tight_layout()
fig.savefig('plots/displacement_vs_target.png')
plt.close(fig)

print("Plots saved to plots/")
print(f"  Running time: grid O(n log n), brute force O(n^2)")
print(f"  Memory: grid {popt_mg[0]:.2f} bytes/vertex overhead, brute force {popt_mb[0]:.2f} bytes/vertex")
print(f"  Speedup at 74k vertices: {times_bf[-1]/times_grid[-2]:.0f}x")
