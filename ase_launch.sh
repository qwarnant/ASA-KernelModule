#!/bin/bash

# Check if the script is launched as root
if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

# Insert the module
insmod ase_cmd.ko

# Start 2 mock process to track
yes > /dev/null &
firstyes=$!

echo "start first yes"
echo $firstyes > /proc/ase_cmd

echo "sleep for 3 seconds"
sleep 3

echo "execution time for $firstyes"
cat /proc/ase/$firstyes

echo "start second yes"
yes > /dev/null &
secyes=$!

echo $secyes > /proc/ase_cmd

echo "sleep for 3 seconds"
sleep 3

echo "execution time for $firstyes"
cat /proc/ase/$firstyes
echo "execution time for $secyes"
cat /proc/ase/$secyes

# Remove the module
rmmod ase_cmd.ko



