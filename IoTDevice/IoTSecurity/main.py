# This program should plot time vs size for each device
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

TIME_BIN = 300*6
# p1 = packet(row['Packet ID'], row['TIME'], row['Size'], row['eth.src'], row['eth.dst'], row['IP.src'], row['IP.dst'], row['IP.proto'], row['port.src'], row['port.dst'])

fileName = "16-09-26"
df = pd.read_csv("data/original/"+fileName+"/"+fileName+".csv")
deviceList_df = pd.read_csv("data/original/devices.txt")

for i, row in deviceList_df.iterrows():

    devName = deviceList_df.iloc[i]['Device']
    devMAC = deviceList_df.iloc[i]['MAC']
    devMAC = ' '.join(devMAC.split())

    figName = devName+".png"
    device_df_t1 = df[df['eth.src'] == devMAC]

    if device_df_t1.empty:
        print("Dataframe of "+devName+"is empty")

    else:
        #########
        device_df_time = device_df_t1[['TIME', 'Size']]
        # device_df_time.TIME = pd.to_numeric(device_df_time.TIME)
        # device_df_time.Size = pd.to_numeric(device_df_time.Size)

        firstTime = device_df_t1.iloc[0]['TIME']
        lastTime = device_df_t1.iloc[-1]['TIME']

        binRange = np.arange(firstTime, lastTime, TIME_BIN)
        cats, bins = pd.cut(device_df_time['TIME'], binRange, retbins=True)
        device_df = device_df_time.groupby(cats).sum()
        final = device_df.drop(columns=['TIME'])
        final.plot.bar()
        plt.savefig("data/original/"+fileName+"/"+figName, bbox_inches='tight')
        print(devName+" completed")

