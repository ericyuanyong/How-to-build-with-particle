import tkinter as tk
import threading
import time
import random

# Global variables for relay state and power measurements
relay_on = False
voltage_value = "0V"
current_value = "0A"
watt_value = "0W"

def update_values():
    global voltage_value, current_value, watt_value
    # Example function to update voltage, current, and watt values
    voltage_value = "120V"
    current_value = "5A"
    watt_value = "600W"

def update_gui_values():
    # Update GUI labels with the latest values
    
    voltage_label.config(text=f"Voltage: {random.randint(1,1000)}")
    current_label.config(text=f"Current: {random.randint(1,1000)}")
    watt_label.config(text=f"Watt: {random.randint(1,1000)}")
    # Schedule the next update after 1 second
    root.after(1000, update_gui_values)

def toggle_relay():
    global relay_on
    relay_on = not relay_on
    if relay_on:
        relay_button.config(text="Relay: ON", bg="green")
        # Add logic to turn on the relay here
        print("Relay turned ON")
    else:
        relay_button.config(text="Relay: OFF", bg="red")
        # Add logic to turn off the relay here
        print("Relay turned OFF")

def simulate_values_update():
    while True:
        # Replace with your actual logic to fetch or calculate values
        update_values()
        time.sleep(1)  # Simulate update every second

# Create the main window
root = tk.Tk()
root.title("Power Measurements and Relay Control")

# Create a frame to hold the output labels
frame = tk.Frame(root, padx=20, pady=20)
frame.pack(padx=10, pady=10)

# Labels for displaying voltage, current, and watt
voltage_label = tk.Label(frame, text="Voltage: ")
voltage_label.pack(anchor=tk.W)

current_label = tk.Label(frame, text="Current: ")
current_label.pack(anchor=tk.W)

watt_label = tk.Label(frame, text="Watt: ")
watt_label.pack(anchor=tk.W)

# Button to update values (just for demonstration)
update_button = tk.Button(root, text="Update Values", command=update_values)
update_button.pack()

# Button to control relay
relay_button = tk.Button(root, text="Relay: OFF", bg="red", command=toggle_relay)
relay_button.pack(pady=10)

# Start a thread to simulate values update
update_thread = threading.Thread(target=simulate_values_update, daemon=True)
update_thread.start()

# Start updating GUI with initial values
update_gui_values()

# Run the main loop
root.mainloop()