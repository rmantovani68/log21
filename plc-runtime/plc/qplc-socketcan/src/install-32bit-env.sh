#!/bin/bash

sudo apt -y install build-essential binutils

# Similarly, we’ll install gcc-multilib package to add the Multilib mechanism to our system:

sudo apt -y install gcc-multilib

# If we’re running Fedora, Redhat, or CentOS, we’ll use dnf to install the gcc compiler, Make, and binutils:

sudo dnf -y install gcc binutils make

# In the same vein, to add 32-bit support for Redhat-like distributions, we run:

sudo dnf -y install glibc-devel.i686
