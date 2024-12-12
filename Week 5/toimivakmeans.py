import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Step 1: Load and preprocess the sensor data (no scaling)
def load_data(csv_file):
    """Load sensor data from a CSV file and select x, y, z columns."""
    df = pd.read_csv(csv_file, sep=';', header=None)
    print(df.head())  # Print the first few rows of the dataframe

    # Select the relevant columns (assuming columns F, G, H are indices 5, 6, 7 in the dataframe)
    data = df.iloc[5:, [5, 6, 7]].to_numpy()

    # Return raw data without scaling
    return data

# Step 2: Implement K-means functions
def initialize_centroids(data, k):
    """Initialize centroids using K-means++."""
    centroids = [data[np.random.randint(0, data.shape[0])]]  # Randomly pick the first centroid
    
    for _ in range(1, k):
        # Compute distances from each point to the nearest selected centroid
        distances = np.min([np.linalg.norm(data - centroid, axis=1) for centroid in centroids], axis=0)
        
        # Handle NaN values in distances by replacing them with a large number (infinity)
        #distances = np.nan_to_num(distances, nan=np.inf)
        
        # Compute probabilities based on distances
        probabilities = distances / np.sum(distances)
        
        # Handle NaN values in probabilities, if any
        probabilities = np.nan_to_num(probabilities, nan=0.0)
        
        # If the sum of probabilities is zero, assign uniform probability to each point
        if np.sum(probabilities) == 0:
            probabilities = np.ones(len(data)) / len(data)

        # Select the next centroid based on the computed probabilities
        next_centroid = data[np.random.choice(range(data.shape[0]), p=probabilities)]
        centroids.append(next_centroid)

    return np.array(centroids)


def assign_clusters(data, centroids):
    """Assign each data point to the nearest centroid."""
    distances = np.linalg.norm(data[:, np.newaxis] - centroids, axis=2)  # Compute distances
    print(f"Distances: {distances}")  # Print distances to debug
    
    clusters = np.argmin(distances, axis=1)  # Assign each data point to the closest centroid
    return clusters

def update_centroids(data, clusters, k, new_centroids):
    """Update centroid positions as the mean of assigned points."""
    updated_centroids = np.array([
        data[clusters == i].mean(axis=0) if np.any(clusters == i) else new_centroids[i]
        for i in range(k)
    ])

    # Debugging: print the updated centroids
    print(f"Updated Centroids: {updated_centroids}")

    # Ensure no empty clusters by reassigning random points to empty clusters
    for i in range(k):
        if np.any(np.isnan(updated_centroids[i])):
            updated_centroids[i] = data[np.random.choice(data.shape[0])]
            print(f"Cluster {i} is empty, reassigning centroid.")
    
    return updated_centroids



def k_means(data, k, max_iterations=100, tolerance=1e-5):
    """Run the K-means clustering algorithm."""
    centroids = initialize_centroids(data, k)
    for iteration in range(max_iterations):
        print(f"\nIteration {iteration + 1}")
        clusters = assign_clusters(data, centroids)
        new_centroids = update_centroids(data, clusters, k, centroids)  # Pass centroids to update function
        
        # Check for convergence
        if np.all(np.abs(new_centroids - centroids) < tolerance):
            print(f"Convergence reached after {iteration + 1} iterations.")
            break
        centroids = new_centroids
    
    return centroids, clusters



def export_to_c_header(centroids, output_file="keskipisteet.h"):
    """Write centroids to a C-style 2D array in a header file."""
    with open(output_file, "w") as f:
        f.write("int CP[6][3] = {\n")
        for i, centroid in enumerate(centroids):
            f.write(f"    {{ {centroid[0]:.4f}, {centroid[1]:.4f}, {centroid[2]:.4f} }}, // Centroid {i+1}\n")
        f.write("};\n")
    print(f"Centroids saved to {output_file}")

def plot_clusters(data, clusters, centroids):
    """Visualize data points and centroids in a 3D scatter plot."""
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')  # Set up 3D plot

    x = len(centroids)
    for i in range(x):
        cluster_points = data[clusters == i]
        ax.scatter(cluster_points[:, 0], cluster_points[:, 1], cluster_points[:, 2], label=f"Cluster {i+1}")
    
    # Plot centroids
    ax.scatter(centroids[:, 0], centroids[:, 1], centroids[:, 2], color='black', marker='x', s=100, label='Centroids')
    
    ax.legend()
    ax.set_title("K-Means Clustering")
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")  # Label for the Z-axis
    plt.show()

def main():
    # Specify input and output files
    input_file = "data.csv"  # Replace with your actual file path
    output_file = "keskipisteet4.h"
    
    # Load and preprocess the data
    data = load_data(input_file)

    # Perform K-means clustering
    k = 6  # Number of clusters
    centroids, clusters = k_means(data, k)  # Get centroids and clusters

    # Export centroids to C header file
    export_to_c_header(centroids, output_file)

    # Optional: Visualize the clustering (3D plot)
    plot_clusters(data, clusters, centroids)

if __name__ == "__main__":
    main()
