#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
from processing import gen_data_matrix

def generate_heatmap(data,x_labels,y_labels,ax=None,
        cbar_kw={},cbarlabel="", **kwargs):
    if not ax:
        ax = plt.gca()

    im = ax.imshow(data,**kwargs)
    cbar = ax.figure.colorbar(im, ax=ax,**cbar_kw)
    cbar.ax.set_ylabel(cbarlabel, rotation=-90, va="bottom")

def generate_scatter(x,y,x_labels,y_labels,ax=None, **kwargs):
    if not ax:
        ax = plt.gca()

    p = ax.scatter(x,y,**kwargs)
    ax.set_xlabel(x_labels)
    ax.set_ylabel(y_labels)

    return ax

def generate_lineplot(x,y,x_labels,y_labels,ax=None, **kwargs):
    if not ax:
        ax = plt.gca()

    p = ax.scatter(x,y,**kwargs)
    ax.set_xlabel(x_labels)
    ax.set_ylabel(y_labels)

    return ax


if __name__ == "__main__":
    print("Gathering data...")
    data = gen_data_matrix()
    print("Plotting data...")
    func_names = np.unique(data['func_name'])
    tids = np.unique(data['tid'])
    tid_cols = ['red','blue','green','orange','purple','cyan','yellow']
    func_cols = ['red','blue','green','orange','purple','cyan','yellow']

    ax = None
    print("TIDS: " + str(tids))

    for i,tid in enumerate(tids):
        data_tid = data[data['tid'] == tid]
        data_tid['time_ms'] -= np.min(data['time_ms'])
        ax = generate_lineplot(data_tid['time_ms'],data_tid['core'],'Time(ms)','Core ID',ax=ax)#,color=tid_cols[i])
        ax = generate_scatter(data_tid['time_ms'],data_tid['core'],'Time(ms)','Core ID',ax=ax,marker='o')#,color=func_cols) # COLOR by function
    if ax is not None:
        ax.legend(tids)
        ax.set_title("thread occupancy across execution")
    plt.show()


# Fill Header
#coreID = True
#for i,item in enumerate(data_mat[0]):
#    if item == '-1':
#        if coreID:
#            data_mat[0][i] = 'CORE_ID'
#        else:
#            data_mat[0][i] = 'TID'
#        coreID = not(coreID)
#
#
#data_mat_tup = []
#for l in data_mat:
#    data_mat_tup.append(tuple(l))
#
#max_threads = maxlen-5
#
#
#data_type_mat = [(data_mat[0][i],np.float64) for i in range(5)] + [('TID'+ str(j//2),np.float64) if j % 2 == 0 else ('CID'+str(j//2-1),np.int16) for j in range(max_threads)]
#dt = np.dtype(data_type_mat)
#headers = data_mat_tup[0]
#del data_mat_tup[0]
#del data_mat[0]
#
#data = np.array(data_mat_tup,dtype=dt)
#
#timestamps = data['Timestamp']/1000 # convert to ms
#timestamps -= timestamps[0]
#end_time = max(timestamps)
#num_ticks = 5
#timestamp_ticks = np.arange(0,end_time + end_time/num_ticks,end_time/num_ticks)
#
#unique_threads,indices = np.unique(data['TID0'],return_inverse=True)
#
#cores = ["Core " + str(i) for i in range(8)]
#
#
#threadsArray = np.array([[int(data[j][i]) for i in range(5,len(data[j]))] for j in range(len(data))])
#
#fig, ax = plt.subplots()
#ax.scatter(timestamps,threadsArray[:,1]) # Kmeans Main Thread Occupancy
#ax.set_yticks(np.arange(9))
#ax.set_yticklabels(cores)
#ax.set_xlabel("Time(ms)")
#ax.set_ylabel("Core ID")
#ax.set_title("Primary Thread Occupancy (Parsec.Kmeans)")
#
#
##heatmapValues = np.zeros((8,len(timestamps))) # shape is 8 rows, timestep columns
##occupiedCores = []
##for t in range(len(timestamps)): # for each timestep
##    unique, counts = np.unique(threadsArray[t,:], return_counts=True)
##    for i,val in enumerate(unique):
##        if (val > -1 and val < 8):
##            heatmapValues[val,t] += counts[i]
##            if not(val in occupiedCores):
##                occupiedCores.append(val)
##occupiedCores = np.sort(occupiedCores)
##coreLabels = [cores[i] for i in occupiedCores]
##
##
##
##im = ax.pcolor(timestamps,np.arange(9),heatmapValues)
##
### Create colorbar
##cbarlabel = 'Number of Threads'
##cbar = ax.figure.colorbar(im, ax=ax)
##cbar.ax.set_ylabel(cbarlabel, rotation=-90, va="bottom")
##
##end_time_rounded = np.round(end_time/1000)*1000
##ax.set_xticks(np.arange(start=0,step=end_time_rounded/10,stop=end_time_rounded))
###ax.set_yticks(np.arange(len(cores)))
##
##print(coreLabels)
### Hide major tick labels
##ax.set_yticklabels('')
### Customize minor tick labels
##ax.set_yticks(occupiedCores+0.5, minor=True)
##ax.set_yticklabels(coreLabels, minor=True)
##
##ax.set_xlabel('Time (ms)')
##ax.set_ylabel('Core ID')
##ax.set_title('Thread Occupancy (Parsec.Kmeans)')
##
#plt.show()
#
##fig.savefig('occupancy-kmeans.png')
