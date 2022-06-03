#!/usr/bin/bash
# Make casper-fs after reboot.
#
# 5th May 2022 script contribution by Paul Weston - paul.w3st0n@googlemail.com

LOGFILE="/home/paul/casper.log"

cd /home/paul/git/casper-fs/module_generator/output

echo "$(date) Running make clean..," 
make clean
echo $?
echo "$(date): Clean finished"

echo "$(date): Running make..."
make
echo $?
echo "$(date): Make finished"

echo "$(date): Inserting module..."
insmod /home/paul/git/casper-fs/module_generator/output/casperfs.ko
echo $?
