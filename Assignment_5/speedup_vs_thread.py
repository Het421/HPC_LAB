import numpy as np
import matplotlib.pyplot as plt

for i in range(1,4):
    data1 = np.loadtxt(f"parallel_immediate_config_{i}.txt", skiprows=1)
    data2 = np.loadtxt(f"parallel_deferred_config_{i}.txt", skiprows=1)

    threads = data1[:,0]
    immediate = data1[:,1]
    deferred = data2[:,1]

    # Speedup (use 2 threads as baseline)
    speedup_im = immediate[0] / immediate
    speedup_def = deferred[0] / deferred

    plt.figure()
    plt.plot(threads, speedup_im, marker='o', label='Immediate')
    plt.plot(threads, speedup_def, marker='s', label='Deferred')

    plt.xlabel("Threads")
    plt.ylabel("Speedup")
    plt.title(f"Speedup vs Threads (Config {i})")
    plt.legend()
    plt.grid()

    plt.savefig(f"speedup_config_{i}.png")

plt.show()