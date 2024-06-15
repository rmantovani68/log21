# PLC

sudo ip link set can0 type can bitrate 500000
ip -details link show can0 
sudo ip link set can0 up

sudo ./go plcv-objs/user.oo

```
roberto@log21:/etc/rc0.d$ ls -l
totale 0
lrwxrwxrwx 1 root root 20 12 mar  2022 K01alsa-utils -> ../init.d/alsa-utils
lrwxrwxrwx 1 root root 22 12 mar  2022 K01avahi-daemon -> ../init.d/avahi-daemon
lrwxrwxrwx 1 root root 19 12 mar  2022 K01bluetooth -> ../init.d/bluetooth
lrwxrwxrwx 1 root root 20 12 mar  2022 K01cryptdisks -> ../init.d/cryptdisks
lrwxrwxrwx 1 root root 22 12 mar  2022 K01cups-browsed -> ../init.d/cups-browsed
lrwxrwxrwx 1 root root 17 12 mar  2022 K01hddtemp -> ../init.d/hddtemp
lrwxrwxrwx 1 root root 20 12 mar  2022 K01irqbalance -> ../init.d/irqbalance
lrwxrwxrwx 1 root root 17 12 mar  2022 K01lightdm -> ../init.d/lightdm
lrwxrwxrwx 1 root root 23 12 mar  2022 K01lvm2-lvmpolld -> ../init.d/lvm2-lvmpolld
lrwxrwxrwx 1 root root 17 12 mar  2022 K01openvpn -> ../init.d/openvpn
lrwxrwxrwx 1 root root 18 12 mar  2022 K01plymouth -> ../init.d/plymouth
lrwxrwxrwx 1 root root 20 20 giu 19.01 K01postgresql -> ../init.d/postgresql
lrwxrwxrwx 1 root root 37 12 mar  2022 K01pulseaudio-enable-autospawn -> ../init.d/pulseaudio-enable-autospawn
lrwxrwxrwx 1 root root 15 12 mar  2022 K01saned -> ../init.d/saned
lrwxrwxrwx 1 root root 27 12 mar  2022 K01speech-dispatcher -> ../init.d/speech-dispatcher
lrwxrwxrwx 1 root root 15 12 mar  2022 K01uuidd -> ../init.d/uuidd
lrwxrwxrwx 1 root root 26 12 mar  2022 K02cryptdisks-early -> ../init.d/cryptdisks-early
lrwxrwxrwx 1 root root 20 12 mar  2022 K02networking -> ../init.d/networking
lrwxrwxrwx 1 root root 17 12 mar  2022 K02rsyslog -> ../init.d/rsyslog
lrwxrwxrwx 1 root root 20 12 mar  2022 K03hwclock.sh -> ../init.d/hwclock.sh
lrwxrwxrwx 1 root root 14 12 mar  2022 K03udev -> ../init.d/udev
lrwxrwxrwx 1 root root 25  1 lug 15.10 K04qplc -> /etc/init.d/start_qplc.sh




lrwxrwxrwx 1 root root 25  1 lug 15.10 K04qplc -> /etc/init.d/start_qplc.sh
```

```
#!/bin/sh
echo "Started on : "`date +%Y-%m-%d-%H-%M-%S`  > /opt/sorter/plc/plc.errors
cd /opt/sorter/plc ; ./go plc-objs/user.oo
```

```
roberto@log21:/etc/systemd/system$ ls -lrt
-rw-r--r-- 1 root root  146  1 lug 15.13 qplc.service
```

```
roberto@log21:/etc/systemd/system$ cat qplc.service
[Unit]
Description=QPLC Startup Script
After=network.target

[Service]
ExecStart=/opt/sorter/plc/start_qplc.sh

[Install]
WantedBy=default.target
```

