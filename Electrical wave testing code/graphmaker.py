import matplotlib.pyplot as plt
import scienceplots
time = []
voltage = []
time2 = []
voltage2 = []

with open("F0012CH1.CSV",'r') as file:
    for line in file:
        line = line.replace(',','')
        line = line.split()
        time.append(float(line[0]))
        voltage.append(float(line[1]))
with open("F0012CH2.CSV", 'r') as file2:
    for line in file2:
        line = line.replace(',','')
        line = line.split()
        time2.append(float(line[0]))
        voltage2.append(float(line[1]))

percentage = 25
plt.style.use(['science','ieee'])
plt.plot(time, voltage, label = f'4.8V')
plt.plot(time2, voltage2, label=f'0.8V')

plt.ylabel(f'Voltage (Volts)')
plt.xlabel('Time (seconds)')
plt.legend(loc='upper right')

plt.xlim(-0.06, 0.06)
plt.ylim(-0.01, 6)
plt.tight_layout()
plt.show()
