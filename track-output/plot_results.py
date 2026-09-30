import pandas as pd
import matplotlib.pyplot as plt
df = pd.read_csv("results.csv")
summary = (
    df.groupby(["n", "m"], as_index=False)
      .agg(
          boruvka_ms=("boruvka_ms", "mean"),
          kruskal_ms=("kruskal_ms", "mean"),
          prim_ms=("prim_ms", "mean"),
          boruvka_phases=("boruvka_phases", "mean"),
      )
)
for m_per_n in sorted((summary["m"] / summary["n"]).unique()):
    subset = summary[(summary["m"] / summary["n"]).round(6) == round(m_per_n, 6)]
    plt.figure()
    plt.plot(subset["n"], subset["boruvka_ms"], marker="o", label="Boruvka")
    plt.plot(subset["n"], subset["kruskal_ms"], marker="o", label="Kruskal")
    plt.plot(subset["n"], subset["prim_ms"], marker="o", label="Prim")
    plt.xlabel("Number of vertices")
    plt.ylabel("Mean runtime (ms)")
    plt.title(f"Runtime vs vertices (m/n = {m_per_n:g})")
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"runtime_density_{m_per_n:g}.png", dpi=200)
    plt.close()
plt.figure()
for m_per_n in sorted((summary["m"] / summary["n"]).unique()):
    subset = summary[(summary["m"] / summary["n"]).round(6) == round(m_per_n, 6)]
    plt.plot(
        subset["n"],
        subset["boruvka_phases"],
        marker="o",
        label=f"m/n = {m_per_n:g}",
    )
plt.xlabel("Number of vertices")
plt.ylabel("Mean Boruvka phases")
plt.title("Boruvka phases vs graph size")
plt.legend()
plt.tight_layout()
plt.savefig("boruvka_phases.png", dpi=200)
plt.close()
