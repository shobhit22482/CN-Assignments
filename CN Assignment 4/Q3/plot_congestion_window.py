import matplotlib.pyplot as plt

with open("tcp-example.cwnd") as file:
    lines = file.readlines()
    
cwnd = []
time_cwnd = []

for line in lines:
    time, start_cwnd, end_cwnd = line.strip().split()
    cwnd.append(int(end_cwnd))
    time_cwnd.append(float(time))
    
plt.plot(time_cwnd, cwnd, label="CWND", color='blue')
plt.xlabel("Time (s)")
plt.ylabel("Congestion Window (CWND) in packets")
plt.title("Congestion Window over Time")
plt.grid(True)
plt.legend()
plt.show()