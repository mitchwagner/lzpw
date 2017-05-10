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

# Time versus threads 
plt.figure(0)
for i in range(0, 5): 
    plt.plot(num_threads_master[i], time_master[i], label=label_master[i])
plt.title("Time to Compress vs. Number of Threads Used (Images)", y=1.08)
plt.xlabel("Number of Threads")
plt.ylabel("Time to Compress (s)")
plt.legend()
plt.savefig("test.pdf") 

# Time versus threads 
plt.figure(1)
for i in range(5, 9): 
    plt.plot(num_threads_master[i], time_master[i], label=label_master[i])
plt.legend()
plt.title("Time to Compress vs. Number of Threads Used (ASCII)", y=1.08)
plt.xlabel("Number of Threads")
plt.ylabel("Time to Compress (s)")
plt.savefig("test2.pdf") 

# Space versus threads 
plt.figure(2)
for i in range(0, 5): 
    plt.plot(num_threads_master[i], filesize_master[i], label=label_master[i])
plt.legend()
plt.title("Compressed File Size vs. Number of Threads Used (Images)", y=1.08)
plt.xlabel("Number of Threads")
plt.ylabel("File Size (bytes)")
plt.savefig("test3.pdf") 

# Space versus threads 
plt.figure(3)
for i in range(5, 9): 
    plt.plot(num_threads_master[i], filesize_master[i], label=label_master[i])
plt.legend()
plt.title("Compressed File Size vs. Number of Threads Used (ASCII)", y=1.08)
plt.xlabel("Number of Threads")
plt.ylabel("File Size (bytes)")
plt.savefig("test4.pdf") 


################################################################################
# Decompression Statistics
################################################################################
# File, threads in, threads out, seconds

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

num_items = len(threads_out_master[0])
x = np.arange(0, 34, 6)
labels = None
for i, header in enumerate(header_master):
    if (i % 6 == 0):
        fig = plt.figure(i + 4)
        ax = fig.gca(projection='3d')
        labels = header_master[i:i+6]
    ax.plot([32 - ((i % 6) * 6 )] * num_items, threads_out_master[i], zs=time_master[i], label=header_master[i])
    
    if (i % 6 == 5):
        plt.subplots_adjust(left=.1,right=.88,bottom=.1,top=.9)
        plt.xticks([])
        ax.set_zlabel("Time (s)")
        ax.set_ylabel("# Threads Used in Decompression")
        leg = ax.legend(loc='lower left')
        plt.title("Time to Decompress vs. # Threads Used \n During Compression/Decompression", y=1.08)
        bb = leg.get_bbox_to_anchor().inverse_transformed(ax.transAxes)
        offset = -.1
        bb.x0 += offset
        bb.x1 += offset
        bb.y0 += offset
        bb.y1 += offset 
        leg.set_bbox_to_anchor(bb, transform = ax.transAxes)
        plt.savefig("images/" + header + ".pdf") 



