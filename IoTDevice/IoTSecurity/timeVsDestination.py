# This program should plot time vs size for each device
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

TIME_BIN = 300*6
# p1 = packet(row['Packet ID'], row['TIME'], row['Size'], row['eth.src'], row['eth.dst'], row['IP.src'], row['IP.dst'], row['IP.proto'], row['port.src'], row['port.dst'])

fileName = "16-09-23"
df = pd.read_csv("data/original/"+fileName+"/"+fileName+".csv")
deviceList_df = pd.read_csv("data/original/devices.txt")

for j, row in deviceList_df.iterrows():

    devName = deviceList_df.iloc[j]['Device']
    devMAC = deviceList_df.iloc[j]['MAC']
    devMAC = ' '.join(devMAC.split())

    logName = devName+"_logFile.txt"
    logName2 = devName+"_IPADDRESSES.txt"

    logfile = open("data/original/"+fileName+"/"+logName, "w+")
    ipAddressFile = open("data/original/"+fileName+"/"+logName2, "w+")

    figName = devName+".png"

    try:
        device_df_t1 = df[df['eth.src'] == devMAC]

        if device_df_t1.empty:
            print("THIS IS EMPTY")

        else:
            device_df_dest = device_df_t1[['TIME', 'IP.dst', 'Size']]
            er1 = device_df_dest.groupby('IP.dst')['TIME'].apply(list)
            numberOfDevices = len(er1)
            ipAddresses = er1.index.tolist()
            print(numberOfDevices)

            i = 0
            while i < numberOfDevices:
                print(j, ': This is repetition: ', i)
                lengthOfList = len(er1[i])
                listOfDummys = np.ones(lengthOfList)+i
                plt.scatter(er1[i], listOfDummys, s=2)
                ipAddressFile.write(str(i)+': '+ipAddresses[i]+'\n')

                i = i+1

            plt.savefig("data/original/"+fileName+"/"+figName, figsize=(20, 18), dpi=200)
            print(devName + " completed")

    except IOError:
        print(devName+': An error occured trying to read the file.')
        logfile.write(devName+': IO error.\n')

    except ValueError:
        print(devName+': Non-numeric data found in the file.')
        logfile.write(devName + ': Value error.\n')

    except ImportError:
        print (devName+': NO module found')
        logfile.write(devName + ': Import error.\n')

    except EOFError:
        print(devName+':Why did you do an EOF on me?')
        logfile.write(devName + ': EOF error.\n')

    except KeyboardInterrupt:
        print(devName+': You cancelled the operation.')
        logfile.write(devName + ': Keyboard interrupt.\n')

    except:
        print(devName+' :An error occured.')
        logfile.write(devName + ': Unknown error.\n')

    logfile.close()
    ipAddressFile.close()

# classify as inside and outside devices
