#!/usr/bin/env python

import csv
import sys
import argparse
import ConfigParser
from pprint import pprint

from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np

config = None
try:
    config = ConfigParser.RawConfigParser()
    config.read('config.conf')
    nFloors = config.getint('CONFIG', 'nFloors')
    xnFlats = config.getint('CONFIG', 'xnFlats')
    ynFlats = config.getint('CONFIG', 'ynFlats')
    nSta = config.getint('CONFIG', 'nSta')
    side = config.get('CONFIG', 'side').split(",")
    showFloor = config.getint('CONFIG', 'showFloor')

    parser = argparse.ArgumentParser()
    parser.add_argument('--nFloors', type=int, dest='nFloors', default=nFloors)
    parser.add_argument('--xnFlats', type=int, dest='xnFlats', default=xnFlats)
    parser.add_argument('--ynFlats', type=int, dest='ynFlats', default=ynFlats)
    parser.add_argument('--nSta', type=int, dest='nSta', default=nSta)
    parser.add_argument('--side', nargs=3, dest='side', default=side)
    parser.add_argument('--showFloor', type=int, dest='showFloor', default=showFloor)
    args = parser.parse_args()
except:
    parser = argparse.ArgumentParser()
    parser.add_argument('--nFloors', type=int, required=True, dest='nFloors')
    parser.add_argument('--xnFlats', type=int, required=True, dest='xnFlats')
    parser.add_argument('--ynFlats', type=int, required=True, dest='ynFlats')
    parser.add_argument('--nSta', type=int, required=True, dest='nSta')
    parser.add_argument('--side', nargs=3, dest='side', default=[False, 0, 0])
    parser.add_argument('--showFloor', type=int, dest='showFloor', default=0)
    args = parser.parse_args()

nFloors = args.nFloors
xnFlats = args.xnFlats
ynFlats = args.ynFlats
nSta = args.nSta
side = args.side
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
if side[0]=='False':
    if ynFlats>xnFlats:
        size = (6.4*(xnFlats/ynFlats), 6.4)
    else:
        size = (6.4, 6.4*(ynFlats/xnFlats))
        fig = plt.figure(figsize=size)
    # ax = fig.add_subplot(111, projection='3d')
    # ax.scatter(APs[0], APs[1], APs[2], c='b', marker='o')
    plt.plot(APs[0][showFloor*xnFlats*ynFlats:(showFloor+1)*xnFlats*ynFlats],
             APs[1][showFloor*xnFlats*ynFlats:(showFloor+1)*xnFlats*ynFlats],
             'bo')
    plt.plot(STAs[0][showFloor*xnFlats*ynFlats*nSta:(showFloor+1)*xnFlats*ynFlats*nSta],
             STAs[1][showFloor*xnFlats*ynFlats*nSta:(showFloor+1)*xnFlats*ynFlats*nSta],
             'r.')
    plt.xticks(map(lambda x : x * 10, list(range(xnFlats+1))))
    plt.yticks(map(lambda x : x * 10, list(range(ynFlats+1))))
    plt.grid(True)
    # ax.set_zticks([0, 3, 6])
    plt.xlim(0, xnFlats*10)
    plt.ylim(0, ynFlats*10)
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.title("Floor: {}".format(showFloor))
    # plt.axis('scaled')
    # ax.set_zlim(0, 6)
    plt.show()
else:
    fig = plt.figure()
    # ax = fig.add_subplot(111, projection='3d')
    # ax.scatter(APs[0], APs[1], APs[2], c='b', marker='o')
    xory = 0
    l = xnFlats
    if side[1]=='y':
        xory = 1
        l = ynFlats
    APnew = [[], []]
    STAnew = [[], []]
    for k in range(len(APs[xory])):
        if k%(xnFlats*ynFlats) in map(lambda x: x+l*int(side[2]), range(l)):
            APnew[0].append(APs[xory][k])
    for k in range(len(APs[2])):
        if k%(xnFlats*ynFlats) in map(lambda x: x+l*int(side[2]), range(l)):
            APnew[1].append(APs[2][k])
    for k in range(len(STAs[xory])):
        if k%(xnFlats*ynFlats*nSta) in map(lambda x: x+l*int(side[2])*nSta, range(l*nSta)):
            STAnew[0].append(STAs[xory][k])
    for k in range(len(STAs[2])):
        if k%(xnFlats*ynFlats*nSta) in map(lambda x: x+l*int(side[2])*nSta, range(l*nSta)):
            STAnew[1].append(STAs[2][k])
    pprint(STAs)
    pprint(STAnew)
    plt.plot(APnew[0],
             APnew[1],
             'bo')
    plt.plot(STAnew[0],
             STAnew[1],
             'r.')
    plt.xticks(map(lambda x : x * 10, list(range(l+1))))
    plt.yticks(map(lambda x : x * 3, list(range(nFloors+1))))
    plt.grid(True)
    # ax.set_zticks([0, 3, 6])
    plt.xlim(0, l*10)
    plt.ylim(0, nFloors*3)
    if side[1]=='y':
        plt.xlabel('Y')
    else:
        plt.xlabel('X')
    plt.ylabel('Z')
    plt.title("Wall: {}".format(side[2]))
    # plt.axis('scaled')
    # ax.set_zlim(0, 6)
    plt.show()
