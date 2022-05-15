import numpy as np
import matplotlib.pyplot as plt

data = np.loadtxt('graph_0.05_16.txt')

plt.plot(data[:,0], data[:,1])
plt.plot(data[:,0], data[:,2])
plt.plot(data[:,0], data[:,3])

plt.show()