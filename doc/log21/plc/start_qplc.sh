#!/bin/sh


sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up


echo "Started on : "`date +%Y-%m-%d-%H-%M-%S`  > /opt/sorter/plc/plc.errors
ip -details link show can0 >> /opt/sorter/plc/plc.errors
cd /opt/sorter/plc ; make clean all ; ./go plc-objs/user.oo >> /opt/sorter/plc/plc.errors
