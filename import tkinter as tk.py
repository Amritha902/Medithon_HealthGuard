import tkinter as tk
from tkinter import messagebox, ttk
from tkcalendar import Calendar
import sqlite3
import serial
import firebase_admin
from firebase_admin import credentials, db
import matplotlib.pyplot as plt
from sklearn.ensemble import RandomForestClassifier
import numpy as np

# Firebase setup
cred = credentials.Certificate(r"C:\Users\amrit\Documents\^o^\PROJECTS\PILL DISPENSER\jjj.json")

firebase_admin.initialize_app(cred, {
    'databaseURL': r"https://tabletdispenser-3fb84-default-rtdb.firebaseio.com/"

})

# Screen setup
root = tk.Tk()
root.title("Tablet Dispensing System")
root.geometry("800x600")

# Global Variables
patient_data = {"name": "", "age": "", "medical_conditions": ""}
calendar_data = {}

### ---- SCREEN 1: Patient Data Input ---- ###
def screen_patient_data():
    clear_window()
    
    def save_patient_data():
        name = name_entry.get()
        age = age_entry.get()
        med_cond = med_entry.get()
        patient_data['name'] = name
        patient_data['age'] = age
        patient_data['medical_conditions'] = med_cond
        messagebox.showinfo("Success", "Patient data saved.")

    # GUI elements
    tk.Label(root, text="Patient Data", font=("Helvetica", 16)).pack(pady=10)
    
    tk.Label(root, text="Name:").pack(pady=5)
    name_entry = tk.Entry(root)
    name_entry.pack()

    tk.Label(root, text="Age:").pack(pady=5)
    age_entry = tk.Entry(root)
    age_entry.pack()

    tk.Label(root, text="Medical Conditions:").pack(pady=5)
    med_entry = tk.Entry(root)
    med_entry.pack()

    tk.Button(root, text="Save", command=save_patient_data).pack(pady=10)
    tk.Button(root, text="Go to Calendar", command=screen_calendar).pack(pady=10)

### ---- SCREEN 2: Calendar ---- ###
def screen_calendar():
    clear_window()

    def add_appointment():
        selected_date = cal.get_date()
        appt_time = time_entry.get()
        calendar_data[selected_date] = appt_time
        messagebox.showinfo("Appointment Saved", f"Appointment set on {selected_date} at {appt_time}")

    def view_tablets():
        selected_date = cal.get_date()
        ref = db.reference(f'/patients/{patient_data["name"]}/tablets/{selected_date}')
        tablets_today = ref.get()
        if tablets_today:
            messagebox.showinfo("Tablets", f"Tablets for {selected_date}: {tablets_today}")
        else:
            messagebox.showinfo("Tablets", "No tablets scheduled for today.")

    tk.Label(root, text="Calendar", font=("Helvetica", 16)).pack(pady=10)
    
    cal = Calendar(root, selectmode="day", year=2024, month=9, day=28)
    cal.pack(pady=20)

    tk.Label(root, text="Appointment Time (HH:MM):").pack(pady=5)
    time_entry = tk.Entry(root)
    time_entry.pack()

    tk.Button(root, text="Add Appointment", command=add_appointment).pack(pady=10)
    tk.Button(root, text="View Tablets Today", command=view_tablets).pack(pady=10)

    tk.Button(root, text="Go to Health Monitor", command=screen_health_monitor).pack(pady=10)

### ---- SCREEN 3: Health Monitoring Dashboard ---- ###
def screen_health_monitor():
    clear_window()

    def fetch_sensor_data():
        # Simulating fetching data from sensors or Firebase
        ref = db.reference(f'/patients/{patient_data["name"]}/health_data')
        sensor_data = ref.get()
        return sensor_data

    def plot_health_data(sensor_data):
        labels = ['Heart Rate', 'Temperature', 'Movement (Acc)']
        values = [sensor_data['heart_rate'], sensor_data['temperature'], sensor_data['accelerometer']]

        plt.bar(labels, values, color=['blue', 'orange', 'green'])
        plt.ylim(0, 150)
        plt.title("Health Monitoring Data")
        plt.show()

    def check_health_status(sensor_data):
        heart_rate_status = "Normal" if 60 < sensor_data['heart_rate'] < 100 else "Abnormal"
        temp_status = "Normal" if 36.5 < sensor_data['temperature'] < 37.5 else "Abnormal"
        acc_status = "Normal" if sensor_data['accelerometer'] < 2 else "Abnormal"
        messagebox.showinfo("Health Status", f"Heart Rate: {heart_rate_status}\nTemp: {temp_status}\nMovement: {acc_status}")

    tk.Label(root, text="Health Monitoring", font=("Helvetica", 16)).pack(pady=10)
    
    sensor_data = fetch_sensor_data()
    
    tk.Button(root, text="Plot Health Data", command=lambda: plot_health_data(sensor_data)).pack(pady=10)
    tk.Button(root, text="Check Health Status", command=lambda: check_health_status(sensor_data)).pack(pady=10)

    tk.Button(root, text="Go to AI Diagnosis", command=screen_ai_diagnosis).pack(pady=10)

### ---- SCREEN 4: AI Diagnosis ---- ###
def screen_ai_diagnosis():
    clear_window()

    # def diagnose():
    #     # Simulate AI diagnosis using heart rate and temperature
    #     sensor_data = fetch_sensor_data()
    #     heart_rate = sensor_data['heart_rate']
    #     temperature = sensor_data['temperature']

    #     # Simple AI diagnostic rule-based model
    #     if heart_rate > 100 and temperature > 37.5:
    #         diagnosis = "You may have a fever."
    #     elif heart_rate < 60:
    #         diagnosis = "You may have bradycardia."
    #     else:
    #         diagnosis = "Your vitals are normal."

    #     messagebox.showinfo("Diagnosis", diagnosis)

    # tk.Label(root, text="AI Diagnosis", font=("Helvetica", 16)).pack(pady=10)
    # tk.Button(root, text="Diagnose", command=diagnose).pack(pady=10)

### ---- Helper Functions ---- ###
def clear_window():
    for widget in root.winfo_children():
        widget.destroy()

# Start at patient data screen
screen_patient_data()

# Run the main loop
root.mainloop()
