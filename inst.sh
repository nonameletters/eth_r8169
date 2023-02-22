#!/bin/bash

rmmod nn_r8169
insmod nn_r8169.ko
ip addr add 192.168.10.23/24 dev eth0
ip link set up dev eth0
