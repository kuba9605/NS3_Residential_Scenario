#!/usr/bin/python

import csv
import sys
import argparse

from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np

parser = argparse.ArgumentParser()
parser.add_argument('--nFloors', type=int, required=True, dest='nFloors')
parser.add_argument('--xnFlats', type=int, required=True, dest='xnFlats')
parser.add_argument('--ynFlats', type=int, required=True, dest='ynFlats')
parser.add_argument('--nSta', type=int, required=True, dest='nSta')
parser.add_argument('--showFloor', type=int, required=True, dest='showFloor')
args = parser.parse_args()

nFloors = args.nFloors
xnFlats = args.xnFlats
ynFlats = args.ynFlats
nSta = args.nSta
showFloor = args.showFloor

csv_file = open('nodes.csv', 'r')
csv_reader = csv.reader(csv_file, delimiter=';')
APs = [[], [], []]
STAs = [[], [], []]
counter = 0;
for row in csv_reader:
    if counter==0:
        pass
    else:
        APs[0].append(float(row[0]))
        APs[1].append(float(row[1]))
        APs[2].append(float(row[2]))
        for i in range(nSta):
            STAs[0].append(float(row[(i+1)*3]))
            STAs[1].append(float(row[(i+1)*3+1]))
            STAs[2].append(float(row[(i+1)*3+2]))
    counter = counter + 1

fig = plt.figure()
# ax = fig.add_subplot(111, projection='3d')
# ax.scatter(APs[0], APs[1], APs[2], c='b', marker='o')
plt.plot(APs[0][showFloor*xnFlats*ynFlats:(showFloor+1)*xnFlats*ynFlats],
         APs[1][showFloor*xnFlats*ynFlats:(showFloor+1)*xnFlats*ynFlats],
         'bo')
plt.plot(STAs[0][showFloor*xnFlats*ynFlats*nSta:(showFloor+1)*xnFlats*ynFlats*nSta],
         STAs[1][showFloor*xnFlats*ynFlats*nSta:(showFloor+1)*xnFlats*ynFlats*nSta],
         'r^')
plt.xticks(map(lambda x : x * 10, list(range(xnFlats))))
plt.yticks(map(lambda x : x * 10, list(range(ynFlats))))
plt.grid(True)
# ax.set_zticks([0, 3, 6])
# ax.set_xlim(0, 20)
# ax.set_ylim(0, 20)
# ax.set_zlim(0, 6)
plt.show()
