import matplotlib.pyplot as plt

enqueue_line = "/NodeList/0/DeviceList/0/$ns3::PointToPointNetDevice/TxQueue/Enqueue"
dequeue_line = "/NodeList/0/DeviceList/0/$ns3::PointToPointNetDevice/TxQueue/Dequeue"

with open("tcp-example.tr") as file:
    lines = file.readlines()

queue = []
queue_delay = []
times = []

for line in lines:
    cur_time = float(line.split()[1])
    
    if enqueue_line in line:
        queue.append(cur_time)
    elif dequeue_line in line and queue:
        delay = cur_time - queue.pop(0)
        queue_delay.append(delay)
        times.append(cur_time)

plt.plot(times, queue_delay, label="Queue Delay", color='red')
plt.xlabel("Time (s)")
plt.ylabel("Queue Delay (s)")
plt.title("Queue Delay over Time")
plt.grid(True)
plt.legend()
plt.show()
