'''
This script is designed to create figures using output from testing script.

Author: Mitch Wagner
Date: May 8, 2017
'''

import csv
import itertools
import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D

################################################################################
# Compression Statistics
################################################################################

label = None
label_master = [] 
num_threads_master = []
time_master = []
filesize_master = []

with open("compress_stats.dat", "rb") as csvfile:
    reader = csv.reader(csvfile)
    
    num_threads = []
    time = [] 
    filesize = []
    for row in reader:
        if row[0] != label:
            if (label is not None):
                num_threads_master.append(num_threads)
                time_master.append(time)
                filesize_master.append(filesize)
                num_threads = []
                time = [] 
                filesize = []
            label_master.append(row[0])
            label = row[0] 
        num_threads.append(row[1])
        time.append(row[2])
        filesize.append(row[3])
    num_threads_master.append(num_threads)
    time_master.append(time)
    filesize_master.append(filesize)

print label_master
print num_threads_master
print time_master
print filesize_master

# Time versus threads 
plt.figure(0)
for i, item in enumerate(label_master):
    plt.plot(num_threads_master[i], time_master[i], label=label_master[i])
plt.legend()
plt.savefig("test.pdf") 

# Space versus threads 
plt.figure(1)
for i, item in enumerate(label_master):
    plt.plot(num_threads_master[i], filesize_master[i], label=label_master[i])
plt.legend()
plt.savefig("test2.pdf") 


################################################################################
# Decompression Statistics
################################################################################
# File, threads in, threads out, seconds
'''
label = None
label_master = []
threads_in_master = []
threads_out_master = []
time_master = []

with open("decompress_stats.dat", "rb") as csvfile:
    reader = csv.reader(csvfile)
    
    threads_in = [] 
    threads_out = []
    time = []
    for row in reader:
        if row[0] != label:
            if (label is not None):
                threads_in_master.append(threads_in)
                threads_out_master.append(threads_out)
                time_master.append(time)
                threads_in = []
                threads_out = []
                time = []
            label_master.append(row[0])
            label = row[0] 
        threads_in.append(row[1])
        threads_out.append(row[2])
        time.append(row[3])
    threads_in_master.append(threads_in)
    threads_out_master.append(threads_out)
    time_master.append(time)
'''

header = None
header_master = []
threads_out_master = []
time_master = []

with open("decompress_stats.dat", "rb") as csvfile:
    reader = csv.reader(csvfile)
    
    threads_out = []
    time = []
    for row in reader:
        if row[0]+"-" + str(row[1]) != header:
            if (header is not None):
                threads_out_master.append(threads_out)
                time_master.append(time)
                threads_out = []
                time = []
            header = row[0]+"-"+str(row[1])
            header_master.append(header)
        threads_out.append(int(row[2]))
        time.append(float(row[3]))
    threads_out_master.append(threads_out)
    time_master.append(time)

print "------------------------------"


num_items = len(threads_out_master[0])
# What I was hoping for was a separate label for every label_thread_in combo
# then the y-axis would be thread_out, and the z-axis would be time
x = np.arange(0, 32, 6)
labels = None
for i, header in enumerate(header_master):
    if (i % 6 == 0):
        fig = plt.figure(i + 3)
        ax = fig.gca(projection='3d')
        labels = header_master[i:i+6]
    ax.plot([(i % 6) * 6] * num_items, threads_out_master[i], zs=time_master[i])
    
    if (i % 6 == 5):
        plt.xticks(x, labels)
        ax.set_xticklabels(ax.xaxis.get_majorticklabels(), rotation=45)
        plt.yticks([1,2,4,6,8,16,32])
        plt.savefig("images/" + header + ".pdf") 
'''
plt.xticks(x, labels)
ax.set_xticklabels(ax.xaxis.get_majorticklabels(), rotation=45)
plt.yticks([1,2,4,6,8,16,32])
plt.savefig("images/" + header_master[len(header_master)-1]+ ".pdf") 
'''





