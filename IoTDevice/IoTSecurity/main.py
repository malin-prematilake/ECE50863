import pandas as pd
import matplotlib.pyplot as plt
import datetime
import csv
from packet import packet

'''
with open('data/testSet.csv', mode='r') as csv_file:
    csv_reader = csv.DictReader(csv_file)
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            line_count += 1

        p1 = packet(row['Packet ID'], row['TIME'], row['Size'], row['eth.src'], row['eth.dst'], row['IP.src'], row['IP.dst'], row['IP.proto'], row['port.src'], row['port.dst'])
        p1.description()
        line_count += 1
    print("Processed {line_count} lines.")
'''

df = pd.read_csv("data/original/16-09-23/16-09-23.csv")
babyMonitor_df = df[df['eth.src'] == "18:b4:30:25:be:e4"]

babyMonitor_df['TIME'] = pd.to_datetime(babyMonitor_df['TIME'], unit='s')

babyMonitor_df.set_index("TIME", inplace=True)
babyMonitor_df.sort_index(inplace=True)
print(babyMonitor_df.head())
babyMonitor_df["Size"].plot.bar()
plt.show()
