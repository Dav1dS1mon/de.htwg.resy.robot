#!/bin/bash

./disable_services.sh 
make
sudo ./motor_test
./uninitialize.sh
./enable_services.sh
