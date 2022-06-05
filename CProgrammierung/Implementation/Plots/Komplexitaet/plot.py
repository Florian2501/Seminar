import os
import numpy as np
import matplotlib.pyplot as plt

cur_path = os.path.dirname(__file__)

new_path = os.path.relpath('pruefenKomp_Mixed.txt', cur_path)

data = np.loadtxt(new_path)

plt.plot(data[:,0], data[:,1], label='Standard BF')
plt.legend()

plt.xlabel("Anzahl Hashfunktionen k")
plt.ylabel("Zeit in s")

plt.show()