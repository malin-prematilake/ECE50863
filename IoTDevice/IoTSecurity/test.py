# This program should plot time vs size for each device
import pandas as pd
from addressLookupTest import *
import matplotlib.pyplot as plt
import numpy as np
import random

TIME_BIN = 300*6
# p1 = packet(row['Packet ID'], row['TIME'], row['Size'], row['eth.src'], row['eth.dst'], row['IP.src'], row['IP.dst'], row['IP.proto'], row['port.src'], row['port.dst'])

fileName = "16-09-29"
df = pd.read_csv("data/original/"+fileName+"/"+fileName+".csv")
deviceList_df = pd.read_csv("data/original/devices.txt")

devIndex = 2  # change this to choose the device interested

devName = deviceList_df.iloc[devIndex]['Device']
devMAC = deviceList_df.iloc[devIndex]['MAC']

devMAC = ' '.join(devMAC.split())

routerMAC = '14:cc:20:51:33:ea'
print(devName, '==', devMAC)

try:
    device_df_t1 = df[df['eth.src'] == devMAC]  # clear
    # device_df_goingOut = device_df_t1[findDomainType(device_df_t1['IP.dst']) == 'foreign']  # clear

    # collecting foreign bound traffic
    destinationsFrgn = [None]
    destinationsFrgnName = [None]
    destinationFrgnSizes = [None]

    destinationsLocal = [None]
    destinationsLocalName = [None]
    destinationLocalSizes = [None]


    for i, row in device_df_t1.iterrows():
        typeD = findDomainType(row['IP.dst'])
        # fix 0 and broadcasting addresses
        if typeD == 'foreign':
            destinationsFrgn.append(row['IP.dst'])
            destinationFrgnSizes.append(row['Size'])
            # destinationsFrgnName.append(findDomainName(row['IP.dst']))

        elif typeD == 'local':
            destinationsLocal.append(row['IP.dst'])
            destinationLocalSizes.append(row['Size'])
        else:
            print('********: ', row['IP.dst'])
            destinationsFrgn.append(row['IP.dst'])
            destinationFrgnSizes.append(row['Size'])

    destinationsFrgn = filter(None, destinationsFrgn)
    destinationsLocal = filter(None, destinationsLocal)
    destinationFrgnSizes = filter(None, destinationFrgnSizes)
    destinationLocalSizes = filter(None, destinationLocalSizes)

    sumOfSizesLocal = sum(destinationLocalSizes)
    sumOfSizesFrgn = sum(destinationFrgnSizes)

    plt.pie([sumOfSizesLocal, sumOfSizesFrgn], labels=['Local', 'Internet'], colors=['Red', 'Blue'], autopct='%1.1f%%',
            shadow=True, startangle=140)
    plt.show()

    print('=============')
    print (sumOfSizesLocal)

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
