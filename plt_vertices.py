import numpy as np
import matplotlib.pyplot as plt

# Predefine three unit directions.
# These can be any 2D unit vectors. Here we choose for example:
directions = np.array([
    [0.1, 0.2],                   # A unit vector pointing right.
    [-np.sqrt(3), 1],         # A unit vector rotated 120° from the horizontal.
    [np.sqrt(7), 1]         # A unit vector rotated -120° from the horizontal.
])

# Define the vertices of the equilateral triangle.
v1 = np.array([0.0, 0.0])
v2 = np.array([1.0, 0.0])
v3 = np.array([0.5, np.sqrt(3) / 2])
vertices = np.array([v1, v2, v3])

# Create a new plot.
fig, ax = plt.subplots(figsize=(6, 6))
ax.set_aspect('equal')

# Plot the equilateral triangle by connecting the vertices.
# Append the first vertex at the end to close the triangle.
triangle = np.vstack((vertices, vertices[0]))
ax.plot(triangle[:, 0], triangle[:, 1], 'b-', linewidth=2, label="Equilateral Triangle 1")

# Plot the predetermined unit vectors on each vertex.
arrow_scale = 0.3  # Adjust arrow length for better visualization

for i, (vertex, direction) in enumerate(zip(vertices, directions)):
    # The arrow starts at the vertex and extends in the direction of the unit vector.
    start_x, start_y = vertex
    vecx,vecy=direction
    dx, dy = np.array([vecx,vecy])/np.linalg.norm(direction,2)
    ax.arrow(start_x, start_y, dx, dy,
             head_width=0.05, head_length=0.07,
             fc='r', ec='r',
             label=f"Direction at V{i+1}" if i == 0 else "")
    # Label the vertices.
    ax.text(start_x + 0.02, start_y + 0.02, f"V{i+1}", fontsize=12, color='darkgreen')

# Optionally, mark the triangle's vertices.
ax.plot(vertices[:, 0], vertices[:, 1], 'ko')

# Set plot title and labels.
ax.set_title("Equilateral Triangle with Predefined Unit Vectors at Each Vertex")
ax.set_xlabel("X-axis")
ax.set_ylabel("Y-axis")
ax.legend()

# Enable grid and display the plot.
plt.grid(True)
plt.savefig("direction1.png")
plt.close()


theta=2*np.pi/3
U=np.array([[np.cos(theta),-np.sin(theta)],[np.sin(theta),np.cos(theta)]])
# Create a new plot.
fig, ax = plt.subplots(figsize=(6, 6))
ax.set_aspect('equal')

# Plot the equilateral triangle by connecting the vertices.
# Append the first vertex at the end to close the triangle.
triangle = np.vstack((vertices, vertices[0]))
ax.plot(triangle[:, 0], triangle[:, 1], 'b-', linewidth=2, label="Equilateral Triangle 2")
for i, (vertex, direction) in enumerate(zip(vertices, directions)):
    start_x, start_y = vertex
    vecx,vecy=direction
    dx, dy = U@np.array([vecx,vecy])/np.linalg.norm(direction,2)
    ax.arrow(start_x, start_y, dx, dy,
             head_width=0.05, head_length=0.07,
             fc='r', ec='r',
             label=f"Direction at V{i+1}" if i == 0 else "")
    # Label the vertices.
    ax.text(start_x + 0.02, start_y + 0.02, f"V{i+1}", fontsize=12, color='darkgreen')

# Optionally, mark the triangle's vertices.
ax.plot(vertices[:, 0], vertices[:, 1], 'ko')

# Set plot title and labels.
ax.set_title("Equilateral Triangle with Predefined Unit Vectors at Each Vertex")
ax.set_xlabel("X-axis")
ax.set_ylabel("Y-axis")
ax.legend()

# Enable grid and display the plot.
plt.grid(True)
plt.savefig("direction2.png")
plt.close()
