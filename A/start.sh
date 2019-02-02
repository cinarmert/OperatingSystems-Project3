make
insmod our_module.ko processid=$1
rmmod our_module
dmesg -c
