import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import sys
import os
import math
from pathlib import Path
# This script plots dipoles for one T, one path

if len(sys.argv) != 4:
    print("wrong number of arguments")
    exit(1)
N = int(sys.argv[1])
TStr = sys.argv[2]

init_path=sys.argv[3]
csvDataFolderRoot = f"../dataAll/N{N}/T{TStr}/csvOutAll/"
dipole_each_site_dir=csvDataFolderRoot+"/dipole_each_site/"
Path(dipole_each_site_dir).mkdir(exist_ok=True,parents=True)
one_path_folder=csvDataFolderRoot+f"/init_path{init_path}/"

dipole_csv_file_name=one_path_folder+"avg_dipole_combined.csv"
if not os.path.exists(dipole_csv_file_name):
    print(f"avg_dipole_combined.csv does not exist for {TStr}")
    exit(1)

dipole_arr = np.array(pd.read_csv(dipole_csv_file_name, header=None))
# The four rows: [Px, Py, Qx, Qy]
Px = dipole_arr[0, :]
Py = dipole_arr[1, :]
Qx = dipole_arr[2, :]
Qy = dipole_arr[3, :]

# Append Qx to Px and Qy to Py
Px_Qx_combined = np.append(Px, Qx)  # Append Qx to Px
Py_Qy_combined = np.append(Py, Qy)  # Append Qy to Py

# Compute the mean of the combined arrays
avg_polarization_x = np.mean(Px_Qx_combined)  # Mean of Px and Qx combined
avg_polarization_y = np.mean(Py_Qy_combined)  # Mean of Py and Qy combined

# Print the results
print(f"Average polarization along x (Px and Qx combined): {avg_polarization_x}")
print(f"Average polarization along y (Py and Qy combined): {avg_polarization_y}")

# Reshape the dipole components
Px_arr = Px.reshape((N, N))
Py_arr = Py.reshape((N, N))
Qx_arr = Qx.reshape((N, N))
Qy_arr = Qy.reshape((N, N))

# Define the lattice constant
a = 2
# Instead of a flat list followed by meshgrid, generate index grids first.
# Let n0 and n1 be the integer indices corresponding to the two lattice directions.
n0 = np.arange(N)
n1 = np.arange(N)

# Use index ordering consistent with how the CSV was written
# For a triangular (non-square) lattice:
i_grid, j_grid = np.meshgrid(n0, n1, indexing="ij")

# Compute positions for sublattice A:
# The formulas below are taken from your original code.
X_O = a * i_grid - 0.5 * a * j_grid
Y_O = (np.sqrt(3) / 2) * a * j_grid


# Define the displacement for sublattice B; adjust these as needed.
D = np.array([0, np.sqrt(3) / 3 * a])
X_D = X_O + D[0]
Y_D = Y_O + D[1]

# Compute the magnitudes for each dipole vector for sublattice A and B
mag_A = np.sqrt(Px_arr**2 + Py_arr**2)
mag_B = np.sqrt(Qx_arr**2 + Qy_arr**2)
mag_min = min(mag_A.min(), mag_B.min())
mag_max = max(mag_A.max(), mag_B.max())

# Plot using quiver; the 5th argument is the color array.
plt.figure(figsize=(90, 60))
scale=0.8

# Plot dipoles for sublattice A with a colormap for the magnitude
qA = plt.quiver(
    X_O, Y_O,
    Px_arr, Py_arr,
    mag_A,
    cmap='viridis',
    scale=scale,
    scale_units='xy',
    angles='xy'
)

# Plot dipoles for sublattice B with the same colormap
qB = plt.quiver(
    X_D, Y_D,
    Qx_arr, Qy_arr,
    mag_B,
    cmap='viridis',
    scale=scale,
    scale_units='xy',
    angles='xy'
)

plt.xlabel("x", fontsize=100)
plt.ylabel("y", fontsize=100)
plt.title(f"Dipole on each site for T = {TStr}, init_path{init_path}", fontsize=120)
plt.axis("equal")

# Add colorbar from one of the quiver plots and increase number size on the colorbar.
cbar = plt.colorbar(qA, label="Dipole Magnitude", fraction=0.046, pad=0.04)
cbar.ax.tick_params(labelsize=120)

plt.savefig(one_path_folder+f"/dipole_each_site_T{TStr}_init_path{init_path}.png")
plt.savefig(dipole_each_site_dir+f"/dipole_each_site_T{TStr}_init_path{init_path}.png")
plt.close()