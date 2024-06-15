# INSTALL

## packages needed

|Name                               | Description                                |
| :--- | :--- |
|gtk+-3.0                           | GTK+ - GTK+ Graphical UI Library           |
|libmemcached                       | libmemcached - libmemcached C/C++ library. |
|libpq                              | libpq - PostgreSQL libpq library           |
|libzmq                             | libzmq - 0MQ c++ library                   |
|ncurses                            | ncurses - ncurses 6.3 library              |

## install on apt based distro - see install-packages-apt in scripts
```
sudo apt-get install libgtk-3-dev 
sudo apt-get install postgresql libpq-dev -y
sudo apt-get install libmemcached-dev -y
sudo apt-get install memcached -y
sudo apt-get install libzmq3-dev -y
sudo apt-get install libncurses-dev   -y
sudo apt-get install unixodbc-dev -y
```

## install on dnf based distro - see install-packages-dnf in scripts
```
sudo dnf install gtk3-devel -y
sudo dnf install postgresql libpq-devel -y
sudo dnf install memcached-devel -y
sudo dnf install memcached -y
sudo dnf install zeromq-devel -y
sudo dnf install ncurses-devel -y
sudo dnf install unixodbc-devel -y
```

## postgresql

### postgresql installation
```
sudo apt-get install postgresql-14 libpq-dev -y
sudo systemctl start postgresql
sudo systemctl enable postgresql
```
enter as postgres user
```
sudo -i -u postgres
```
create supervisor user and exit
```
createuser -s roberto
exit
```
create mr database
```
createdb mr
```


### pg_hba.conf
```
# Database administrative login by Unix domain socket
local   all             postgres                                peer

# TYPE  DATABASE        USER            ADDRESS                 METHOD

# "local" is for Unix domain socket connections only
local   all             all                                     peer
# IPv4 local connections:
host    all             all             127.0.0.1/32            trust
# IPv6 local connections:
host    all             all             ::1/128                 trust
# Allow replication connections from localhost, by a user with the
# replication privilege.
local   replication     all                                     peer
host    replication     all             127.0.0.1/32            scram-sha-256
host    replication     all             ::1/128                 scram-sha-256
```

### postgresql.conf
```
listen_addresses = '*'                  # what IP address(es) to listen on;
```

### restart
```
sudo systemctl restart postgresql
```

### pg_admin
```
sudo apt-get install python3-virtualenv libpq-dev python3-dev
source venv/bin/activate
pip install https://ftp.postgresql.org/pub/pgadmin/pgadmin4/v7.1/pip/pgadmin4-7.1-py3-none-any.whl
```

edit the file:
```
sudo vi venv/lib/python3.10/site-packages/pgadmin4/config_local.py
```

insert the text:

```
import os
DATA_DIR = os.path.realpath(os.path.expanduser(u'~/.pgadmin/'))
LOG_FILE = os.path.join(DATA_DIR, 'pgadmin4.log')
SQLITE_PATH = os.path.join(DATA_DIR, 'pgadmin4.db')
SESSION_DB_PATH = os.path.join(DATA_DIR, 'sessions')
STORAGE_DIR = os.path.join(DATA_DIR, 'storage')
SERVER_MODE = False
```

exec and navigate to localhost:5050

```
python venv/lib/python3.10/site-packages/pgadmin4/pgAdmin4.py
```
### dbf2sql

Viene utilizzato dallo script crea_tabelle.sh

```
make all && sudo make install
```

## edit_plc / plc2 / plc

install i686 environment and libxt i386 (for plc2)
fedora
```
sudo yum -y install glibc.i686
sudo dnf install libXt.i686
sudo yum -y install glibc-devel.i686 libstdc++-devel.i686
```

ubuntu
```
sudo dpkg --add-architecture i386

sudo apt-get update
sudo apt-get install libc6:i386 libncurses5:i386 libstdc++6:i386
```

go in edit_plc folder
run install
sudo ./install.sh


