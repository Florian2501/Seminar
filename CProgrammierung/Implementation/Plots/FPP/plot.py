import os
import numpy as np
import matplotlib.pyplot as plt

cur_path = os.path.dirname(__file__)

new_path = os.path.relpath('graph_0.05_16.txt', cur_path)

data = np.loadtxt(new_path)

plt.plot(data[:,0], data[:,1], label='Standard BF')
plt.plot(data[:,0], data[:,2], label='Floom-Filter')
plt.plot(data[:,0], data[:,3], label='Big BF')
plt.legend()

plt.xlabel("Anzahl Elemente")
plt.ylabel("Falsch-Positiv-Rate")

plt.show()