## dependencies

### debian 
```
sudo apt-get install build-essential -y
sudo apt-get install libgtk-3-dev -y
sudo apt-get install libpq-dev -y
sudo apt-get install libzmq3-dev -y
sudo apt-get install libmemcached-dev -y
sudo apt-get install libcurses-dev -y
sudo apt-get install unixodbc-dev -y -y

```


### fedora 
```
sudo dnf install gtk3*
sudo dnf install gtk4*
sudo dnf install glade
sudo dnf install ncurses-devel
sudo dnf install libmemcached-devel
sudo dnf install postgresql-devel
sudo dnf install postgresql-server
sudo dnf install postgresql-libs
sudo dnf install zeromq-devel
```

## how to connect via ssh to old computers:
```
sh -oHostKeyAlgorithms=+ssh-rsa  alberto@zevio
```

