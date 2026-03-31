"""
HPC Assignment 05 – Plotting Script
Generates all required plots for Experiment 01 and Experiment 02.

Experiment 01 (Serial): Execution Time vs Particle Count (log-log scale)
  - One figure per grid configuration (3 total)
  - Each figure compares Deferred vs Immediate on Lab PC and HPC Cluster

Experiment 02 (Parallel): Speedup vs Number of Threads
  - One figure per grid configuration (3 total)
  - Each figure compares Deferred vs Immediate on Lab PC and HPC Cluster
"""

import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
import os

# os.makedirs("hpc_lab_5", exist_ok=True)

# ─────────────────────────────────────────────────────────
# DATA
# ─────────────────────────────────────────────────────────

particles = [100, 10_000, 1_000_000, 10_000_000, 100_000_000]
threads   = [2, 4, 8, 16]

serial = {
    "cluster": {
        "deferred": {
            1: [0.000113, 0.006644, 0.508273, 4.910448, 49.846617],
            2: [0.000061, 0.005032, 0.511344, 5.070622, 51.755009],
            3: [0.000067, 0.006417, 0.516221, 5.138233, 58.522817],
        },
        "immediate": {
            1: [0.000070, 0.006760, 0.496451, 4.093966, 40.182380],
            2: [0.000043, 0.004129, 0.409784, 4.206814, 44.973317],
            3: [0.000057, 0.005249, 0.431379, 4.281520, 42.903916],
        },
    },
    "pc": {
        "deferred": {
            1: [0.000122, 0.010734, 0.284754, 2.840240, 28.986221],
            2: [0.000032, 0.003102, 0.300897, 3.000772, 29.301937],
            3: [0.000041, 0.003453, 0.348290, 3.339596, 32.286265],
        },
        "immediate": {
            1: [0.000107, 0.009933, 0.286908, 2.734156, 26.846799],
            2: [0.000030, 0.002956, 0.285840, 2.732923, 27.704524],
            3: [0.000042, 0.003294, 0.315155, 3.102754, 30.056485],
        },
    },
}

parallel = {
    "cluster": {
        "deferred": {
            1: [5.140839, 4.471303, 4.010141, 3.676160],
            2: [6.422947, 5.715822, 4.527734, 3.838318],
            3: [7.291223, 5.677355, 5.062310, 6.362972],
        },
        "immediate": {
            1: [4.173239, 3.660094, 3.462455, 3.307947],
            2: [4.721617, 3.994306, 3.664671, 3.550396],
            3: [4.421053, 3.986929, 3.729498, 3.629065],
        },
    },
    "pc": {
        "deferred": {
            1: [1.312252, 1.109012, 1.124782, 1.125135],
            2: [1.274247, 1.111223, 1.110720, 1.096953],
            3: [1.666840, 1.563029, 1.524553, 1.548536],
        },
        "immediate": {
            1: [1.059472, 0.881291, 0.903805, 0.869816],
            2: [1.053700, 0.874723, 0.864927, 0.876502],
            3: [1.461636, 1.270627, 1.273352, 1.252735],
        },
    },
}

grid_labels = {
    1: "Config 1 (Nx=250, Ny=100)",
    2: "Config 2 (Nx=500, Ny=200)",
    3: "Config 3 (Nx=1000, Ny=400)",
}

COLOR_DEF = "#1f77b4"   # blue  – Deferred Insertion
COLOR_IMM = "#d62728"   # red   – Immediate Replacement
COLOR_CLU = "#2ca02c"   # green – HPC Cluster
COLOR_PC  = "#ff7f0e"   # orange – Lab PC
COLOR_IDEAL = "#7f7f7f"

# ─────────────────────────────────────────────────────────
# EXPERIMENT 01 – Serial: Execution Time vs Particle Count
# One figure per grid config.  Each figure has 2 side-by-side subplots
#   Left:  Lab PC    Right: HPC Cluster
# Each subplot has 2 lines: Deferred (blue) and Immediate (red)
# ─────────────────────────────────────────────────────────

print("Generating serial scaling plots …")
for cfg in [1, 2, 3]:
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    fig.suptitle(
        f"Experiment 01 – Serial Execution Time vs Particle Count\n{grid_labels[cfg]}",
        fontsize=13, fontweight="bold",
    )

    for ax, system, sys_label in zip(
        axes, ["pc", "cluster"], ["Lab PC", "HPC Cluster"]
    ):
        t_def = serial[system]["deferred"][cfg]
        t_imm = serial[system]["immediate"][cfg]

        ax.loglog(particles, t_def, "o-",  color=COLOR_DEF, lw=2, ms=6,
                  label="Deferred Insertion")
        ax.loglog(particles, t_imm, "s--", color=COLOR_IMM, lw=2, ms=6,
                  label="Immediate Replacement")

        # O(N) reference
        x_ref = np.array([particles[0], particles[-1]], dtype=float)
        y_ref = t_def[0] * (x_ref / particles[0])
        ax.loglog(x_ref, y_ref, "k:", lw=1.2, alpha=0.45, label="O(N) reference")

        ax.set_title(sys_label, fontsize=11)
        ax.set_xlabel("Number of Particles (log scale)", fontsize=10)
        ax.set_ylabel("Total Execution Time (s, log scale)", fontsize=10)
        ax.grid(True, which="both", ls=":", alpha=0.55)
        ax.legend(fontsize=9)

        # nice x-axis labels
        ax.xaxis.set_major_formatter(
            ticker.FuncFormatter(lambda val, _: f"$10^{{{int(np.log10(val))}}}$")
        )

    plt.tight_layout()
    path = f"hpc_lab_5/exp01_serial_cfg{cfg}.png"
    plt.savefig(path, dpi=150, bbox_inches="tight")
    plt.close()
    print(f"  Saved: {path}")

# ─────────────────────────────────────────────────────────
# EXPERIMENT 02 – Parallel: Speedup vs Threads
# One figure per grid config.  2 subplots: Lab PC | HPC Cluster
# Each subplot has 2 speedup curves + ideal speedup
# Speedup computed relative to T=2 baseline.
# ─────────────────────────────────────────────────────────

def compute_speedup(times):
    base = times[0]   # time at 2 threads
    return [base / t for t in times]


print("\nGenerating per-config speedup plots …")
for cfg in [1, 2, 3]:
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    fig.suptitle(
        f"Experiment 02 – Speedup vs Number of Threads (14M Particles)\n{grid_labels[cfg]}",
        fontsize=13, fontweight="bold",
    )

    for ax, system, sys_label in zip(
        axes, ["pc", "cluster"], ["Lab PC", "HPC Cluster"]
    ):
        sp_def = compute_speedup(parallel[system]["deferred"][cfg])
        sp_imm = compute_speedup(parallel[system]["immediate"][cfg])
        ideal  = [t / 2.0 for t in threads]   # ideal speedup relative to 2-thread

        ax.plot(threads, ideal,  "k--", lw=1.5, alpha=0.55, label="Ideal Speedup")
        ax.plot(threads, sp_def, "o-",  color=COLOR_DEF, lw=2, ms=7,
                label="Deferred Insertion")
        ax.plot(threads, sp_imm, "s-",  color=COLOR_IMM, lw=2, ms=7,
                label="Immediate Replacement")

        ax.set_title(sys_label, fontsize=11)
        ax.set_xlabel("Number of Threads", fontsize=10)
        ax.set_ylabel("Speedup (relative to 2 threads)", fontsize=10)
        ax.set_xticks(threads)
        ax.set_xlim(1.5, 17)
        ax.grid(True, ls=":", alpha=0.55)
        ax.legend(fontsize=9)

    plt.tight_layout()
    path = f"hpc_lab_5/exp02_speedup_cfg{cfg}.png"
    plt.savefig(path, dpi=150, bbox_inches="tight")
    plt.close()
    print(f"  Saved: {path}")


# ─────────────────────────────────────────────────────────
# BONUS – Summary: all 3 configs on one plot per approach
# ─────────────────────────────────────────────────────────

print("\nGenerating summary speedup plot …")
for system, sys_label in [("pc", "Lab PC"), ("cluster", "HPC Cluster")]:
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    fig.suptitle(
        f"Speedup Summary – {sys_label} (All Configurations, 14M Particles)",
        fontsize=13, fontweight="bold",
    )

    colors3 = ["#1f77b4", "#ff7f0e", "#2ca02c"]
    markers = ["o", "s", "^"]

    for ax, approach, approach_label in zip(
        axes, ["deferred", "immediate"], ["Deferred Insertion", "Immediate Replacement"]
    ):
        ideal = [t / 2.0 for t in threads]
        ax.plot(threads, ideal, "k--", lw=1.5, alpha=0.5, label="Ideal")

        for cfg, color, marker in zip([1, 2, 3], colors3, markers):
            sp = compute_speedup(parallel[system][approach][cfg])
            ax.plot(threads, sp, f"{marker}-", color=color, lw=2, ms=7,
                    label=grid_labels[cfg])

        ax.set_title(approach_label, fontsize=11)
        ax.set_xlabel("Number of Threads", fontsize=10)
        ax.set_ylabel("Speedup", fontsize=10)
        ax.set_xticks(threads)
        ax.grid(True, ls=":", alpha=0.55)
        ax.legend(fontsize=8)

    plt.tight_layout()
    path = f"hpc_lab_5/exp02_summary_{system}.png"
    plt.savefig(path, dpi=150, bbox_inches="tight")
    plt.close()
    print(f"  Saved: {path}")

print("\nAll plots saved successfully.")