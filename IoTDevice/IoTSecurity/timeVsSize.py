# This program should plot time vs size for each device
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

TIME_BIN = 300*6
# p1 = packet(row['Packet ID'], row['TIME'], row['Size'], row['eth.src'], row['eth.dst'], row['IP.src'], row['IP.dst'], row['IP.proto'], row['port.src'], row['port.dst'])


fileName = "16-10-12"
logName = "logFile.txt"

logfile = open("data/original/"+fileName+"/"+logName, "w+")
df = pd.read_csv("data/original/"+fileName+"/"+fileName+".csv")
deviceList_df = pd.read_csv("data/original/devices.txt")

for i, row in deviceList_df.iterrows():

    devName = deviceList_df.iloc[i]['Device']
    devMAC = deviceList_df.iloc[i]['MAC']
    devMAC = ' '.join(devMAC.split())

    figName = devName + ".png"

    try:
        device_df_t1 = df[df['eth.src'] == devMAC]

        if device_df_t1.empty:
            print("Dataframe of "+devName+" is empty")
            logfile.write(("Dataframe of "+devName+" is empty.\n"))

        else:
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
print("End of program")