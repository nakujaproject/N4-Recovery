import matplotlib.pyplot as plt
import matplotlib.cbook as cbook
import numpy as np
import os

def plot_data():
    # plot the data once the is done running

    if(os.path.exists("sensor-data.csv")):
        print("File found")
    else:
        print("File does not exist")

    # read file
    arr = np.loadtxt("sensor-data.csv", delimiter=",", unpack=True)

    plt.plot(arr[0], arr[1])
    plt.title("Unfiltered x acceleration")
    plt.xlabel("Time (ms)")
    plt.ylabel("X acceleration (m/s^2)")

    plt.show()

plot_data()
