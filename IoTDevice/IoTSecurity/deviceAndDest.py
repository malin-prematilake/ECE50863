# This program should plot time vs size for each device
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import random

TIME_BIN = 300*6
# p1 = packet(row['Packet ID'], row['TIME'], row['Size'], row['eth.src'], row['eth.dst'], row['IP.src'], row['IP.dst'], row['IP.proto'], row['port.src'], row['port.dst'])

fileName = "16-09-23"
df = pd.read_csv("data/original/"+fileName+"/"+fileName+".csv")
deviceList_df = pd.read_csv("data/original/devices.txt")

devINdex = 10
devName = deviceList_df.iloc[devINdex]['Device']
devMAC = deviceList_df.iloc[devINdex]['MAC']

devMAC = ' '.join(devMAC.split())

print(devName, '==', devMAC)

try:
    device_df_t1 = df[df['eth.src'] == devMAC]  # clear
    # if ethSrc(other than router) is in among the device list then it is the ip is in network,

    if device_df_t1.empty:
        print("THIS IS EMPTY")

    else:
        device_df_dest = device_df_t1[['IP.dst', 'TIME', 'Size']]
        groupTime = device_df_dest.groupby('IP.dst')['TIME'].apply(list) # clear
        groupSize = device_df_dest.groupby('IP.dst')['Size'].apply(list) # clear

        ind1 = 0
        more20count = 1
        numberOfDevicesOver20 = 5
        color = ["#" + ''.join([random.choice('0123456789ABCDEF') for j in range(6)])
                 for i in range(numberOfDevicesOver20)]

        numberOfCommunications = np.zeros(numberOfDevicesOver20)
        nameOfCommunications = ["" for x in range(numberOfDevicesOver20)]

        print(numberOfCommunications)
        for row in groupTime:
            if len(row) >= 20:
                numberOfCommunications[more20count-1] = len(row)
                plt.subplot(numberOfDevicesOver20, 1, more20count) # -----------------> count the number of subplots needed insteaad of five
                plt.scatter(np.asarray(row), np.asarray(groupSize[ind1]), color=color[more20count-1], alpha=1, s=3)
                plt.title(groupSize.index[ind1])
                nameOfCommunications[more20count-1] = groupSize.index[ind1]
                more20count = more20count+1

            ind1 = ind1 + 1

        plt.legend(loc='upper right')
        plt.show() # savefig('test1.png', format='png', dpi=1000)

        # pie chart to show the amount of contact
        plt.pie(numberOfCommunications, labels=nameOfCommunications, colors=color, autopct='%1.1f%%', shadow=True,
                startangle=140)
        plt.show()

        ind1 = 0
        for row2 in groupTime:
            if len(row2) < 20:
                plt.scatter(np.asarray(row2), np.asarray(groupSize[ind1]), alpha=1, label=groupSize.index[ind1], s=2)
                # print(ind1, 'Size: ', groupSize[ind1])
            ind1 = ind1 + 1
        plt.legend(loc='upper right')
        plt.show() # savefig('test2.png', format='png', dpi=1000)


except IOError:
    print(devName + ': An error occured trying to read the file.')
    # logfile.write(devName + ': IO error.\n')

except ValueError:
    print(devName + ': Non-numeric data found in the file.')
    # logfile.write(devName + ': Value error.\n')

except ImportError:
    print (devName + ': NO module found')
    # logfile.write(devName + ': Import error.\n')

except EOFError:
    print(devName + ':Why did you do an EOF on me?')
    # logfile.write(devName + ': EOF error.\n')

except KeyboardInterrupt:
    print(devName + ': You cancelled the operation.')
    # logfile.write(devName + ': Keyboard interrupt.\n')

# except:
#     print(devName + ' :An error occured.')
    # logfile.write(devName + ': Unknown error.\n')
