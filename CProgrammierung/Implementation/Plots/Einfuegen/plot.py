import os
import numpy as np
import matplotlib.pyplot as plt

cur_path = os.path.dirname(__file__)

new_path = os.path.relpath('einfuegen_0.01_8.txt', cur_path)

data = np.loadtxt(new_path)

plt.plot(data[:,0], data[:,1])
plt.plot(data[:,0], data[:,2])
plt.plot(data[:,0], data[:,3])

plt.show()