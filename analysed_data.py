import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Read the CSV file into a DataFrame
df = pd.read_csv("C:/Users/HP/Downloads/datacrucial.csv")

# Separate the data into three columns
heart_rate = df['HeartRate']
spo2 = df['SpO2']
temperature = df['Temperature']

# Plot Heart Rate
plt.figure(figsize=(6, 4))
plt.plot(heart_rate, color='red')
plt.title('Heart Rate')
plt.xlabel('Time')
plt.ylabel('Heart Rate')
plt.show(block=False)

# Plot SpO2
plt.figure(figsize=(6, 4))
plt.plot(spo2, color='blue')
plt.title('SpO2')
plt.xlabel('Time')
plt.ylabel('SpO2')
plt.show(block=False)

# Plot Temperature
plt.figure(figsize=(6, 4))
plt.plot(temperature, color='green')
plt.title('Temperature')
plt.xlabel('Time')
plt.ylabel('Temperature (°F)')
plt.show(block=False)

# Descriptive Statistics
plt.figure(figsize=(8, 6))
df.describe().transpose().plot(kind='bar')
plt.title('Descriptive Statistics')
plt.show(block=False)

# Correlation Matrix
plt.figure(figsize=(8, 6))
sns.heatmap(df.corr(), annot=True, cmap='coolwarm', linewidths=0.5)
plt.title('Correlation Matrix')
plt.show(block=False)
