#!/bin/bash

sleep 10
rm -f /home/rozhlas/send_ip/ifconfig_rozhlas.txt
ifconfig >/home/rozhlas/send_ip/ifconfig_rozhlas.txt
until scp /home/rozhlas/send_ip/ifconfig_rozhlas.txt your_user@your_server_that_is_online:
do
   sleep 5
   ifconfig >/home/rozhlas/send_ip/ifconfig_rozhlas.txt
done 

