import serial
import matplotlib.pyplot as plt
import numpy as np

# Change COM port to match yours (e.g., "COM5" on Windows, "/dev/ttyUSB0" on Linux)
ser = serial.Serial("COM9", 115200)

x_vals, y_true_vals, y_pred_vals = [], [], []
plt.ion()  # interactive mode on

while True:
    try:
        line = ser.readline().decode().strip()
        if not line:
            continue
        x, y_true, y_pred = map(float, line.split(","))
        x_vals.append(x)
        y_true_vals.append(y_true)
        y_pred_vals.append(y_pred)

        plt.clf()
        plt.plot(x_vals, y_true_vals, label="True sin(x)")
        plt.plot(x_vals, y_pred_vals, '--', label="Predicted")
        plt.xlabel("x")
        plt.ylabel("y")
        plt.title("Sine Model Live Predictions")
        plt.legend()
        plt.pause(0.05)
    except Exception as e:
        print("Error:", e)
        continue
